#pragma once

#include <boost/dll/shared_library.hpp>
#include <cvs/common/configuration.hpp>
#include <cvs/common/factory.hpp>

#include <memory>

namespace cvs::pipeline {

class IModule {
 public:
  virtual ~IModule()                        = default;
  virtual std::string name() const          = 0;
  virtual int         version() const       = 0;
  virtual void        registerTypes() const = 0;
  virtual bool        checkCompatibility() const {
    return common::Factory::libVersion == common::Factory::kVersion && libVersion == kVersion;
  }

 protected:
  // ABI version
  // Must be incremented with every change in interface layout or its inline functions
  static constexpr int kVersion = 0;
  static const int     libVersion;
};

namespace detail {
using IModuleConstructor = IModule *();
using IModuleDestructor  = void(IModule *);
}  // namespace detail

using IModulePtr  = std::shared_ptr<IModule>;
using IModuleUPtr = std::unique_ptr<IModule, std::function<detail::IModuleDestructor>>;

// Returns nullptr on failure
IModuleUPtr makeModule(const boost::dll::shared_library &lib);

}  // namespace cvs::pipeline
