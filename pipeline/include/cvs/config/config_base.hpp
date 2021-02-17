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

template <auto &name, typename Parent, typename... Values_types>
class Config_static_object {
  static std::optional<Parent> parse(const boost::property_tree::ptree &source) {
    source.get_value_optional<Subtype>(); // TODO: finish
  }
};

// TODO: fix (use name)
template <class Subtype, auto &name>
class Config_static_value {
  auto parse(const boost::property_tree::ptree &source) {
    source.get_value_optional<Subtype>();
  }
};

template <class Subtype, class Parent, size_t index>
class Config_value {
  Subtype _value;

  // static std::optional<

  operator const Subtype &() const {
    return _value;
  }
};

template <size_t dummy_head, class Accumulating_tuple, size_t dummy_tail = 0>
class Dummy {
  static constexpr size_t value = dummy_head;
  using Tuple = Accumulating_tuple;
};

template <typename... Tuples>
using Concatenate_tuples = decltype(std::tuple_cat(std::declval<Tuples>()...));

#define Value(type, name) 0> Dummy_##name; \
  public:                     \
    Config_value<type, Self, Dummy_##name::value> name; \
  protected:                  \
//    inline static Config_static_value<type> static##name { register_field<Config_value<type>>() };\
    typedef Dummy<Dummy_##name::value + 1, \
      Concatenate_tuples<Dummy_##name::Tuple, std::tuple<Config_static_value<type, #name> > >

// TODO: fix tuple forwarding
#define CONFIG_OBJECT(name, ...) \
  struct name {    \
    private: \
      static constexpr size_t _number_of_types = count_arguments(__VA_OPT__(#__VA_ARGS__)); \
  __VA_OPT__(                  \
    protected:                 \
      typedef Dummy<0, std::tuple<> __VA_ARGS__ 0> Dummy_tail;                   \
    public:                      \
      using Parser = Config_static_object<"", name, Dummy_tail::Tuple> \
  )                           \
  };