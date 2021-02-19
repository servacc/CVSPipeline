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

struct Config_base {
  static auto parse(const boost::property_tree::ptree &source) {

  }
};

template <auto& name, typename Types, size_t types_number>
struct Config_static_object {
  //static constexpr auto _name = name;
  //protected:
  template <class Tuple, size_t... indexes>
  static auto helper(const boost::property_tree::ptree &object, std::index_sequence<indexes...>) {
    return std::make_tuple(std::tuple_element<indexes, Tuple>::type::parse(object)...);
  }

  using Parse_return_type =
  std::optional<decltype(helper<Types>(
      std::declval<boost::property_tree::ptree>(),
      std::make_index_sequence<types_number>{}
  ))>;

  static Parse_return_type parse(const boost::property_tree::ptree &source) {
    // if name is empty, then it's a root object
    if constexpr (std::size(name) > 0) {
      const auto &object = source.get_child_optional(name);
      if (!object) {
        // TODO: logs
        return std::nullopt;
      }

      return std::make_optional(helper<Types>(object.get(), std::make_index_sequence<types_number>{}));
    }
    else {
      return std::make_optional(helper<Types>(source, std::make_index_sequence<types_number>{}));
    }
  }
};

// TODO: fix (use name)
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

template <class Subtype, size_t index>
class Config_value {
  Subtype _value;

  // static std::optional<

  operator const Subtype &() const {
    return _value;
  }
};

template <size_t dummy_head, class Accumulating_tuple, size_t dummy_tail = 0>
struct Dummy {
  static constexpr size_t value = dummy_head;
  using Tuple = Accumulating_tuple;
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
    Config_value<type, Dummy_##name::value> _##name; \
  protected:                               \
    static constexpr char const name##_name[] = #name; \
    typedef Dummy<Dummy_##name::value + 1, \
      Concatenate_tuples<Dummy_##name::Parsers, std::tuple<Config_static_value<type, name##_name> > >, \
      Concatenate_tuples<Dummy_##name::Pointers, std::tuple<Self::Field_pointer<Config_value<type, Dummy_##name::value>, &Self::_##name> > >


#define Object_main_part(name, type_suffix, is_name_string_empty, ...) \
  __VA_OPT__(                    \
    protected:                   \
      using Self = name##type_suffix;                           \
      template <class Value, Value name##type_suffix::* pointer> \
      struct Field_pointer { static constexpr Value name##type_suffix::* ptr = pointer; }; \
                   \
      typedef Dummy<0, std::tuple<>, std::tuple<>, __VA_ARGS__, 0> Dummy_tail; \
      static constexpr auto name##_name = get_name<is_name_string_empty>(#name);                          \
    public:\
      using Parsers = Config_static_object<name##_name, Dummy_tail::Parsers>; \
      using Pointers = Dummy_tail::Pointers; \
  )

#define Object(object_name, ...)  0> Dummy_##object_name; \
  protected:                                          \
    struct object_name##_type {    \
      Object_main_part(object_name, _type, false, __VA_ARGS__)                          \
    };                                      \
  public:                                 \
    object_name##_type _##object_name;                    \
    \
  protected:                                    \
    typedef Dummy<Dummy_##object_name::value + 1, \
      Concatenate_tuples<Dummy_##object_name::Parsers, std::tuple<object_name##_type::Parsers> >, \
      Concatenate_tuples<Dummy_##object_name::Pointers, std::tuple<Self::Field_pointer<object_name##_type, &Self::_##object_name> > >


#define Config_object(name, ...) \
  struct name {    \
    Object_main_part(name,, true, __VA_ARGS__)                         \
  };