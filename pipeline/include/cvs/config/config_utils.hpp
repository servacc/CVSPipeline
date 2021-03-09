#pragma once

namespace Utils {

template <typename... Tuples>
using ConcatenateTuples = decltype(std::tuple_cat(std::declval<Tuples>()...));

template <bool erase, size_t string_size>
constexpr auto get_name(const char (&string)[string_size]) {
  if constexpr (erase) {
    return "";
  }
  else {
    return string;
  }
}

constexpr size_t length(const char *str) {
  return (*str == 0) ? 0 : length(str + 1) + 1;
}

template <class Type, bool is_optional = false>
using OptionalWrapper = typename std::conditional<is_optional, std::optional<Type>, Type >::type;

template <typename T, typename Enable = void>
struct Is_optional : std::false_type {};

template <typename T>
struct Is_optional<std::optional<T> > : std::true_type {};

template <class Wrapping_type>
class OptionalKind {
  const Wrapping_type& _reference;

 public:
  explicit OptionalKind(const Wrapping_type& reference) : _reference(reference) {};

  // clang-format off
  [[nodiscard]] bool has_value() const {
    return _reference.has_value();
  // clang-format on
  }

  const Wrapping_type& value() const {
    return _reference;
  }
};

template <class Wrapping_type>
auto toOptionalKind(const Wrapping_type& value) {
  if constexpr (Is_optional<Wrapping_type>::value) {
    return value;
  }
  else {
    return OptionalKind(value);
  }
}

}