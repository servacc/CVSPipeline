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

#define Value(type, name) 0> Dummy_##name; \
  public:                     \
    Config_value<type, Dummy_##name::value> name; \
  protected:                               \
    static constexpr char const name##_name[] = #name;                                      \
    typedef Dummy<Dummy_##name::value + 1, \
      Concatenate_tuples<Dummy_##name::Tuple, std::tuple<Config_static_value<type, name##_name> > >

#define Object(name, ...)  0> Dummy_##name; \
  protected:                                          \
    struct name##_type {    \
    __VA_OPT__(                  \
      protected:                 \
        typedef Dummy<0, std::tuple<>, __VA_ARGS__, 0> Dummy_tail; \
        static constexpr char const name##_name[] = #name;             \
      public:                                                      \
        using Parser = Config_static_object<name##_name, Dummy_tail::Tuple, Dummy_tail::value>; \
    )                           \
    };                                      \
    static constexpr char const name##_name[] = #name;                                      \
    typedef Dummy<Dummy_##name::value + 1, \
      Concatenate_tuples<Dummy_##name::Tuple, std::tuple<name##_type::Parser> >


#define Config_object(name, ...) \
  struct name {    \
  __VA_OPT__(                  \
    protected:                 \
      typedef Dummy<0, std::tuple<>, __VA_ARGS__, 0> Dummy_tail; \
      static constexpr char const name##_name[] = "";             \
    public:                                                      \
      using Parser = Config_static_object<name##_name, Dummy_tail::Tuple, Dummy_tail::value>; \
  )                           \
  };