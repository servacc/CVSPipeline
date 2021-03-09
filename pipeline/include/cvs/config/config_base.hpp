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
#include <boost/property_tree/json_parser.hpp>

#include "config_utils.hpp"
#include "config.h"


class ConfigBase {
  template <typename Pointers, size_t index, typename Object_type, typename Data>
  static constexpr void helper(Object_type& object, Data&& data) {
    using TargetType = typename std::tuple_element<index, Pointers>::type::Value_type;
    auto& target = object.*std::tuple_element<index, Pointers>::type::ptr;
    const auto& data_value = std::get<index>(data);

    if constexpr (Utils::Is_optional<TargetType>::value) {
      target =
        Utils::toOptionalKind(data_value).has_value() ?
        TargetType(Utils::toOptionalKind(data_value).value()) :
        std::nullopt;
    }
    else {
      target = TargetType(data_value);
    }
  }

  template <typename Pointers, typename Object_type, typename Data, std::size_t... indexes>
  static constexpr void makeFromTupleImpl(Object_type& object, Data&& data, std::index_sequence<indexes...>) {
    return (helper<Pointers, indexes>(object, data), ...);
  }

 public:

  template <typename Pointers, typename Object_type, typename Data>
  static constexpr void makeFromTuple(Object_type& object, Data&& data) {
    return makeFromTupleImpl<Pointers>(
      object,
      std::forward<Data>(data),
      std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Data>>>{}
    );
  }

 protected:

  template <typename Parser>
  static auto parse(const boost::property_tree::ptree &source) {
    return Parser::parse(source);
  }

 public:
  template <typename Result>
  static std::optional<Result> make(const boost::property_tree::ptree &source) {
    auto data = parse<typename Result::Parsers>(source);
    if (!data) {
      return std::nullopt;
    }

    return Result(std::move(data.value()));
  }
};

template <auto& name, typename Types, bool is_optional = false>
struct ConfigStaticObject {
 protected:
  template <typename Tuple, size_t... indexes>
  using ResultIntermediateType =
    Utils::OptionalWrapper<decltype(std::make_tuple(
      Utils::toOptionalKind(
        std::tuple_element<indexes, Tuple>::type::parse(std::declval<boost::property_tree::ptree>())
      ).value()...
    )), is_optional>;

  template <typename Tuple, size_t... indexes>
  using ResultType = std::optional<ResultIntermediateType<Types, indexes...> >;

  template <typename Tuple, size_t... indexes>
  static
    ResultType<Types, indexes...>
      helper(const boost::property_tree::ptree &object, std::index_sequence<indexes...>) {

    auto result = std::make_tuple(std::tuple_element<indexes, Tuple>::type::parse(object)...);
    if ((std::get<indexes>(result).has_value() && ...)) {
      return
        std::make_optional(
          std::make_tuple(
            Utils::toOptionalKind(std::get<indexes>(result)).value()...
          )
        );
    }
    else {
      return ResultIntermediateType<Types, indexes...>{};
    }
  }

 public:
  using ParseReturnType =
  decltype(helper<Types>(
      std::declval<boost::property_tree::ptree>(),
      std::make_index_sequence<std::tuple_size<Types>::value>{}
  ));

  static ParseReturnType parse(const boost::property_tree::ptree &source) {
    // if name is empty, then it's a root object
    if constexpr (Utils::length(name) > 0) {
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

enum class ConfigValueKind {
  BASE,
  OPTIONAL,
  WITH_DEFAULT_VALUE,
};

template <typename Subtype, auto &name, ConfigValueKind value_type, typename DefaultValueType = void>
struct ConfigStaticValue {
  static_assert(
    !std::is_const_v<Subtype> && !std::is_reference_v<Subtype>,
    "Subtype must not be a constant or a reference"
  );

  using ResultType = Utils::OptionalWrapper<Subtype, (value_type == ConfigValueKind::OPTIONAL)>;

  static std::optional<ResultType> parse(const boost::property_tree::ptree &source) {
    const auto& value = source.get_optional<Subtype>(name);
    std::optional<Subtype> result;
    if (!value) {
      // TODO: logs
      result = std::nullopt;
    }
    else {
      result = value.get();
    }

    if constexpr (value_type == ConfigValueKind::OPTIONAL) {
      return std::make_optional(result);
    }
    else if constexpr (value_type == ConfigValueKind::WITH_DEFAULT_VALUE) {
      static_assert(!std::is_same<DefaultValueType, void>::value, "Default_value_type must be provided");
      static_assert(
          std::is_convertible<decltype(DefaultValueType::value), Subtype>::value,
          "Can't convert Default_value_type::value to Config_value_type"
      );

      return result ? result.value() : DefaultValueType::value;
    }
    else {
      return result;
    }
  }
};

template <auto &name, ConfigValueKind value_type>
struct ConfigStaticValue<Config, name, value_type, void> {

  static_assert(value_type != ConfigValueKind::WITH_DEFAULT_VALUE, "ConfigStaticValue cannot be of kind DEFAULT");
  using ResultType = Utils::OptionalWrapper<Config, (value_type == ConfigValueKind::OPTIONAL)>;

  static ResultType parse(const boost::property_tree::ptree &source) {
    const auto &object = source.get_child_optional(name);
    Config result((boost::property_tree::ptree()));
    if (object) {
      // TODO: logs
      result = std::move(Config(object.get(), name));
    }

    if constexpr (value_type == ConfigValueKind::OPTIONAL) {
      return std::make_optional(std::move(result));
    }
    else {
      return std::move(result);
    }
  }
};

template <typename ParentType, typename ParsersTuple, typename FieldsPointersTuple, size_t dummy_tail = 0>
struct Dummy {
  using Parent = ParentType;
  using Parsers = ParsersTuple;
  using Pointers = FieldsPointersTuple;
};

#define CONFIG_COMMA ,

#define VALUE_BASE(name, type, config_value_kind, default_declaration, default_type) 0> Dummy_##name; \
  protected:                                                                \
    default_declaration                                                           \
    static constexpr char const name##_name[] = #name;                      \
    using Config_static_type_##name = ConfigStaticValue<type, name##_name, config_value_kind default_type>; \
  public:                     \
    Config_static_type_##name ::ResultType _##name; \
    typedef Dummy<                                           \
      Dummy_##name::Parent, \
      Utils::ConcatenateTuples<Dummy_##name::Parsers, std::tuple<Config_static_type_##name > >, \
      Utils::ConcatenateTuples<                                    \
        Dummy_##name::Pointers,             \
        std::tuple<Self::FieldPointer<Config_static_type_##name ::ResultType, &Self::_##name> \
      >                                                      \
    >

#define VALUE(name, type) VALUE_BASE(name, type, ConfigValueKind::BASE,,)
#define VALUE_OPTIONAL(name, type) VALUE_BASE(name, type, ConfigValueKind::OPTIONAL,,)
#define VALUE_DEFAULT(name, type, default_value) VALUE_BASE(name, type, ConfigValueKind::WITH_DEFAULT_VALUE, \
  struct Default_value_##name_type { static constexpr auto value = default_value; }; , \
  CONFIG_COMMA Default_value_##name_type)


#define OBJECT_MAIN_PART(name, type_suffix, is_name_string_empty, is_optional, ...) \
  __VA_OPT__(                                                          \
      friend Dummy_##name ::Parent; \
      friend ConfigBase;                                              \
    protected:                                                         \
        \
      using Self = name##type_suffix;                           \
      template <typename Value, Value name##type_suffix::* pointer> \
      struct FieldPointer { using Value_type = Value; static constexpr Value name##type_suffix::* ptr = pointer; }; \
                   \
      typedef Dummy<name##type_suffix, std::tuple<>, std::tuple<>, __VA_ARGS__, 0> Dummy_tail; \
      static constexpr auto name##_name = Utils::getName<is_name_string_empty>(#name);                          \
      name##type_suffix() = default;                                                       \
                                                                                    \
      using Parsers = ConfigStaticObject<name##_name, Dummy_tail::Parsers, is_optional>;              \
      using Pointers = Dummy_tail::Pointers;                           \
    public:                                                            \
      template <typename Tuple>                                                               \
      explicit name##type_suffix(Tuple arguments) {                                                \
        ConfigBase::makeFromTuple<Pointers>(*this, arguments);     \
      } \
  )

#define OBJECT_BASE(object_name, is_optional, ...)  0> Dummy_##object_name; \
  protected:                                          \
    struct object_name##_type {                           \
      OBJECT_MAIN_PART(object_name, _type, false, is_optional, __VA_ARGS__) \
    };                                      \
  public:                                 \
    Utils::OptionalWrapper<object_name##_type, is_optional> _##object_name;                    \
    \
  protected:                                    \
    typedef Dummy<                                                          \
      Self, \
      Utils::ConcatenateTuples<Dummy_##object_name::Parsers, std::tuple<object_name##_type::Parsers> >, \
      Utils::ConcatenateTuples<                                                   \
        Dummy_##object_name::Pointers,                                      \
        std::tuple<Self::FieldPointer<Utils::OptionalWrapper<object_name##_type, is_optional>, &Self::_##object_name> \
      >                                                                     \
    >

#define OBJECT_OPTIONAL(object_name, ...) OBJECT_BASE(object_name, true, __VA_ARGS__)
#define OBJECT(object_name, ...) OBJECT_BASE(object_name, false, __VA_ARGS__)

#define CONFIG_OBJECT(name, ...) \
  class name {                  \
    using Dummy_##name = Dummy<name, void, void, 0>; \
    OBJECT_MAIN_PART(name,, true, false, __VA_ARGS__) \
   public:\
    static std::optional<name> make(const boost::property_tree::ptree &source) {   \
      return ConfigBase::make<name> (source);       \
    }                                                                                        \
  };