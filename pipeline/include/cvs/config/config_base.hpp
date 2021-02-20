#pragma once

#include <string>
#include <functional>
#include <type_traits>
#include <utility>
#include <list>
#include <tuple>
#include <set>
#include <memory>
#include <optional>
#include <variant>

#include <boost/property_tree/ptree.hpp>


constexpr size_t length(const char* string) {
  return (*string == 0) ? 0 : length(string + 1) + 1;
}

class Config_base {
  template <class Pointers, size_t index, class Object_type, class Data>
  static constexpr void helper(Object_type& object, Data&& data) {
    object.*std::tuple_element<index, Pointers>::type::ptr =
        typename std::tuple_element<index, Pointers>::type::Value_type(std::get<index>(std::forward<Data>(data)));
  }

  template <class Pointers, class Object_type, class Data, std::size_t... indexes>
  static constexpr void make_from_tuple_impl(Object_type& object, Data&& data, std::index_sequence<indexes...>) {
    return (helper<Pointers, indexes>(object, data), ...);
  }

 public:

  template <class Pointers, class Object_type, class Data>
  static constexpr void make_from_tuple_aggregate(Object_type& object, Data&& data) {
    return make_from_tuple_impl<Pointers>(
        object,
        std::forward<Data>(data),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Data>>>{}
    );
  }


 protected:

  template <class Parser>
  static auto parse(const boost::property_tree::ptree &source) {
    return Parser::parse(source);
  }

 public:
  template <class Result>
  static std::optional<Result> make(const boost::property_tree::ptree &source) {
    auto data = parse<typename Result::Parsers>(source);
    if (!data) {
      return std::nullopt;
    }

    return Result(std::move(data.value()));
  }
};

template <auto& name, typename Types>
struct Config_static_object {
 protected:
  template <class Tuple, size_t... indexes>
  using Result_type =
  std::optional<decltype(std::make_tuple(
      std::tuple_element<indexes, Tuple>::type::parse(std::declval<boost::property_tree::ptree>()).value()...
  ))>;

  template <class Tuple, size_t... indexes>
  static
  Result_type<Types, indexes...>
  helper(const boost::property_tree::ptree &object, std::index_sequence<indexes...>) {

    auto result = std::make_tuple(std::tuple_element<indexes, Tuple>::type::parse(object)...);
    if ((std::get<indexes>(result) && ...)) {
      return std::make_optional(std::make_tuple(std::get<indexes>(result).value()...));
    }
    else {
      return std::nullopt;
    }
  }

 public:
  using Parse_return_type =
  decltype(helper<Types>(
      std::declval<boost::property_tree::ptree>(),
      std::make_index_sequence<std::tuple_size<Types>::value>{}
  ));


  static Parse_return_type parse(const boost::property_tree::ptree &source) {
    // if name is empty, then it's a root object
    if constexpr (length(name) > 0) {
      const auto &object = source.get_child_optional(name);
      if (!object) {
        // TODO: logs
        return std::nullopt;
      }

      return helper<Types>(object.get(), std::make_index_sequence<std::tuple_size<Types>::value>{});
    }
    else {
      return helper<Types>(source, std::make_index_sequence<std::tuple_size<Types>::value>{});
    }
  }
};

template <class Subtype, auto &name>
struct Config_static_value {
  static std::optional<Subtype> parse(const boost::property_tree::ptree &source) {
    const auto& value = source.get_optional<Subtype>(name);
    if (!value) {
      // TODO: logs
      return std::nullopt;
    }

    return value.get();
  }
};

template <class Parent_type, class Parsers_tuple, class Fields_pointers_tuple, size_t dummy_tail = 0>
struct Dummy {
  using Parent = Parent_type;
  using Parsers = Parsers_tuple;
  using Pointers = Fields_pointers_tuple;
};

template <typename... Tuples>
using Concatenate_tuples = decltype(std::tuple_cat(std::declval<Tuples>()...));

template <bool erase, size_t string_size>
constexpr auto get_name(const char (&string)[string_size]) {
  if constexpr (erase) {
    return "";
  }
  else {
    return string;
  }
}

#define Value(type, name) 0> Dummy_##name; \
  public:                     \
    type _##name; \
  protected:                               \
    static constexpr char const name##_name[] = #name; \
    typedef Dummy<Dummy_##name::Parent, \
      Concatenate_tuples<Dummy_##name::Parsers, std::tuple<Config_static_value<type, name##_name> > >, \
      Concatenate_tuples<Dummy_##name::Pointers, std::tuple<Self::Field_pointer<type, &Self::_##name> > >


#define Object_main_part(name, type_suffix, is_name_string_empty, ...) \
  __VA_OPT__(                                                          \
      friend Dummy_##name ::Parent; \
      friend Config_base;                                              \
    protected:                                                         \
        \
      using Self = name##type_suffix;                           \
      template <class Value, Value name##type_suffix::* pointer> \
      struct Field_pointer { using Value_type = Value; static constexpr Value name##type_suffix::* ptr = pointer; }; \
                   \
      typedef Dummy<name##type_suffix, std::tuple<>, std::tuple<>, __VA_ARGS__, 0> Dummy_tail; \
      static constexpr auto name##_name = get_name<is_name_string_empty>(#name);                          \
      name##type_suffix() = default;                                                       \
                                                                                    \
      using Parsers = Config_static_object<name##_name, Dummy_tail::Parsers>;              \
      using Pointers = Dummy_tail::Pointers;                           \
    public:                                                            \
      template <class Tuple>                                                               \
      name##type_suffix(Tuple arguments) {                                                \
        Config_base::make_from_tuple_aggregate<Pointers>(*this, arguments);     \
      } \
  )

#define Object(object_name, ...)  0> Dummy_##object_name; \
  protected:                                          \
    struct object_name##_type {                           \
      Object_main_part(object_name, _type, false, __VA_ARGS__) \
    };                                      \
  public:                                 \
    object_name##_type _##object_name;                    \
    \
  protected:                                    \
    typedef Dummy<Self, \
      Concatenate_tuples<Dummy_##object_name::Parsers, std::tuple<object_name##_type::Parsers> >, \
      Concatenate_tuples<Dummy_##object_name::Pointers, std::tuple<Self::Field_pointer<object_name##_type, &Self::_##object_name> > >


#define Config_object(name, ...) \
  class name {                  \
    using Dummy_##name = Dummy<name, void, void, 0>; \
    Object_main_part(name,, true, __VA_ARGS__) \
   public:\
    static const std::optional<name> parse_and_make(const boost::property_tree::ptree &source) {   \
      return Config_base::make<name> (source);       \
    }                                                                                        \
  };