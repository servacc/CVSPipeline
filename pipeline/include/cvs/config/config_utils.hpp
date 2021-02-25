#pragma once

namespace Utils {

constexpr size_t length(const char *str) {
  return (*str == 0) ? 0 : length(str + 1) + 1;
}

template <class Type, bool is_optional = false>
using Optional_wrapper = typename std::conditional<is_optional, std::optional<Type>, Type >::type;

template <typename T, typename Enable = void>
struct Is_optional : std::false_type {};

template <typename T>
struct Is_optional<std::optional<T> > : std::true_type {};

}