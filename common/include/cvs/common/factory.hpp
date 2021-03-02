#pragma once

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>

namespace cvs::common {

class Factory {
 public:
  template <typename Iface, typename FactoryFunction, typename KeyType>
  static void registrateDefault(const KeyType &key) {
    auto fun = DefaultFactoryFunctionHelper<FactoryFunction>::template createFunction<Iface>();
    registrate(key, std::move(fun));
  }

  // ABI version
  // Must be incremented with every change in class Factory layout or its inline functions
  static constexpr int kVersion = 0;
  static const int     libVersion;
  template <typename Iface, typename FactoryFunction, typename KeyType>
  static bool registrateDefaultIf(const KeyType &key) {
    auto fun = DefaultFactoryFunctionHelper<FactoryFunction>::template createFunction<Iface>();
    return registrateIf(key, std::move(fun));
  }

  template <typename FactoryFunction, typename KeyType>
  static void registrate(const KeyType &key, std::function<FactoryFunction> fun) {
    factoryFunctionsMap<KeyType, FactoryFunction>()[key] = std::move(fun);
  }

  template <typename FactoryFunction, typename KeyType>
  static bool registrateIf(const KeyType &key, std::function<FactoryFunction> fun) {
    if (!isRegistered<FactoryFunction>(key)) {
      registrate(key, std::move(fun));
      return true;
    }

    return false;
  }

  template <typename T, typename KeyType, typename... Args>
  static T create(const KeyType &key, Args... args) {
    auto &creator_map = factoryFunctionsMap<KeyType, T(Args...)>();
    if (auto iter = creator_map.find(key); iter != creator_map.end())
      return iter->second(std::forward<Args>(args)...);

    return {};
  }

  template <typename FactoryFunction, typename KeyType>
  static bool isRegistered(const KeyType &key) {
    auto &creator_map = factoryFunctionsMap<KeyType, FactoryFunction>();
    return creator_map.find(key) != creator_map.end();
  }

 private:
  template <typename FacFunction>
  struct DefaultFactoryFunctionHelper;

  template <typename Res, typename... Args>
  struct DefaultFactoryFunctionHelper<Res(Args...)> {
    template <typename Iface>
    static auto createFunction() {
      return std::function(
          [](Args... args) -> std::unique_ptr<Iface> { return std::make_unique<Res>(std::forward<Args>(args)...); });
    }
  };

  template <typename KeyType, typename FactoryFunction>
  static auto &factoryFunctionsMap() {
    static std::map<KeyType, std::function<FactoryFunction>> map;
    return map;
  }
};

}  // namespace cvs::common
