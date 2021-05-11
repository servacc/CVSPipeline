#pragma once

#include <boost/dll/shared_library.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/pipeline/version.hpp>

#include <memory>

namespace cvs::pipeline {

class IModule {
 public:
  virtual ~IModule()                                                                    = default;
  virtual std::string name() const                                                      = 0;
  virtual int         version() const                                                   = 0;
  virtual void        registerTypes(const cvs::common::FactoryPtr<std::string> &) const = 0;
  virtual bool        checkCompatibility() const;

 protected:
  static const int libVersion;
};

namespace detail {
using IModuleCreator = IModule *();
using IModuleDeleter = void(IModule *);
using IModuleVersion = unsigned int();
}  // namespace detail

using IModulePtr  = std::shared_ptr<IModule>;
using IModuleUPtr = std::unique_ptr<IModule, std::function<detail::IModuleDeleter>>;

// Returns nullptr on failure
IModuleUPtr makeModule(const boost::dll::shared_library &lib);

}  // namespace cvs::pipeline

#define REGISTER_MODULE(module)                                                                 \
  extern "C" cvs::pipeline::IModule *newModule() { return new module; }                         \
  extern "C" void                    deleteModule(cvs::pipeline::IModule *ptr) { delete ptr; }  \
  extern "C" unsigned int            moduleVersionMajor() { return CVSPipeline_VERSION_MAJOR; } \
  extern "C" unsigned int            moduleVersionMinor() { return CVSPipeline_VERSION_MINOR; } \
  extern "C" unsigned int            moduleVersionPatch() { return CVSPipeline_VERSION_PATCH; }
