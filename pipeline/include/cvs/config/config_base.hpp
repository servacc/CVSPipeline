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

#include "config_utils.hpp"


class Config_base {
  template <class Pointers, size_t index, class Object_type, class Data>
  static constexpr void helper(Object_type& object, Data&& data) {
    using Target_type = typename std::tuple_element<index, Pointers>::type::Value_type;
    auto& target = object.*std::tuple_element<index, Pointers>::type::ptr;
    const auto& data_value = std::get<index>(data);

    if constexpr (Utils::Is_optional<Target_type>::value) {
      target = data_value ? Target_type(data_value.value()) : std::nullopt;
    }
    else {
      target = Target_type(data_value);
    }
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

template <auto& name, typename Types, bool is_optional = false>
struct Config_static_object {
 protected:
  template <class Tuple, size_t... indexes>
  using Result_intermediate_type =
    Utils::Optional_wrapper<decltype(std::make_tuple(
      std::tuple_element<indexes, Tuple>::type::parse(std::declval<boost::property_tree::ptree>()).value()...
    )), is_optional>;

  template <class Tuple, size_t... indexes>
  using Result_type = std::optional<Result_intermediate_type<Types, indexes...> >;

  template <class Tuple, size_t... indexes>
  static
    Result_type<Types, indexes...>
      helper(const boost::property_tree::ptree &object, std::index_sequence<indexes...>) {

    auto result = std::make_tuple(std::tuple_element<indexes, Tuple>::type::parse(object)...);
    if ((std::get<indexes>(result) && ...)) {
      return std::make_optional(std::make_tuple(std::get<indexes>(result).value()...));
    }
    else {
      return Result_type<Types, indexes...>(Result_intermediate_type<Types, indexes...>{});
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

enum class Config_value_kind {
  BASE,
  OPTIONAL,
  WITH_DEFAULT_VALUE,
};

template <class Subtype, auto &name, Config_value_kind value_type, class Default_value_type = void>
struct Config_static_value {
  using Result_type = Utils::Optional_wrapper<Subtype, (value_type == Config_value_kind::OPTIONAL)>;

  static std::optional<Result_type> parse(const boost::property_tree::ptree &source) {
    const auto& value = source.get_optional<Subtype>(name);
    std::optional<Subtype> result;
    if (!value) {
      // TODO: logs
      result = std::nullopt;
    }
    else {
      result = value.get();
    }

    if constexpr (value_type == Config_value_kind::OPTIONAL) {
      return std::make_optional(result);
    }
    else if constexpr (value_type == Config_value_kind::WITH_DEFAULT_VALUE) {
      static_assert(!std::is_same<Default_value_type, void>::value, "Default_value_type must be provided");
      static_assert(
          std::is_convertible<decltype(Default_value_type::value), Subtype>::value,
          "Can't convert Default_value_type::value to Config_value_type"
      );

      return result ? result.value() : Default_value_type::value;
    }
    else {
      return result;
    }
  }
};

template <class Parent_type, class Parsers_tuple, class Fields_pointers_tuple, size_t dummy_tail = 0>
struct Dummy {
  using Parent = Parent_type;
  using Parsers = Parsers_tuple;
  using Pointers = Fields_pointers_tuple;
};

#define CONFIG_COMMA ,

#define Value_base(name, type, config_value_kind, default_declaration, default_type) 0> Dummy_##name; \
  protected:                                                                \
    default_declaration                                                           \
    static constexpr char const name##_name[] = #name;                      \
    using Config_static_type_##name = Config_static_value<type, name##_name, config_value_kind default_type>; \
  public:                     \
    Config_static_type_##name ::Result_type _##name; \
    typedef Dummy<                                           \
      Dummy_##name::Parent, \
      Concatenate_tuples<Dummy_##name::Parsers, std::tuple<Config_static_type_##name > >, \
      Concatenate_tuples<                                    \
        Dummy_##name::Pointers,             \
        std::tuple<Self::Field_pointer<Config_static_type_##name ::Result_type, &Self::_##name> \
      >                                                      \
    >

#define Value(name, type) Value_base(name, type, Config_value_kind::BASE,,)
#define Value_optional(name, type) Value_base(name, type, Config_value_kind::OPTIONAL,,)
#define Value_default(name, type, default_value) Value_base(name, type, Config_value_kind::WITH_DEFAULT_VALUE, \
  struct Default_value_##name_type { static constexpr auto value = default_value; }; , \
  CONFIG_COMMA Default_value_##name_type)


#define Object_main_part(name, type_suffix, is_name_string_empty, is_optional, ...) \
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
      static constexpr auto name##_name = Utils::get_name<is_name_string_empty>(#name);                          \
      name##type_suffix() = default;                                                       \
                                                                                    \
      using Parsers = Config_static_object<name##_name, Dummy_tail::Parsers, is_optional>;              \
      using Pointers = Dummy_tail::Pointers;                           \
    public:                                                            \
      template <class Tuple>                                                               \
      name##type_suffix(Tuple arguments) {                                                \
        Config_base::make_from_tuple_aggregate<Pointers>(*this, arguments);     \
      } \
  )

#define Object_base(object_name, is_optional, ...)  0> Dummy_##object_name; \
  protected:                                          \
    struct object_name##_type {                           \
      Object_main_part(object_name, _type, false, is_optional, __VA_ARGS__) \
    };                                      \
  public:                                 \
    Utils::Optional_wrapper<object_name##_type, is_optional> _##object_name;                    \
    \
  protected:                                    \
    typedef Dummy<                                                          \
      Self, \
      Utils::Concatenate_tuples<Dummy_##object_name::Parsers, std::tuple<object_name##_type::Parsers> >, \
      Utils::Concatenate_tuples<                                                   \
        Dummy_##object_name::Pointers,                                      \
        std::tuple<Self::Field_pointer<Utils::Optional_wrapper<object_name##_type, is_optional>, &Self::_##object_name> \
      >                                                                     \
    >

#define Object_optional(object_name, ...) Object_base(object_name, true, __VA_ARGS__)
#define Object(object_name, ...) Object_base(object_name, false, __VA_ARGS__)

#define Config_object(name, ...) \
  class name {                  \
    using Dummy_##name = Dummy<name, void, void, 0>; \
    Object_main_part(name,, true, false, __VA_ARGS__) \
   public:\
    static const std::optional<name> parse_and_make(const boost::property_tree::ptree &source) {   \
      return Config_base::make<name> (source);       \
    }                                                                                        \
  };