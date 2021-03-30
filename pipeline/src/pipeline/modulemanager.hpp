#pragma once

#include <boost/dll.hpp>
#include <cvs/common/factory.hpp>
#include <cvs/logger/loggable.hpp>
#include <cvs/pipeline/imodule.hpp>
#include <cvs/pipeline/imodulemanager.hpp>

#include <map>

namespace cvs::pipeline::impl {

using LibraryUPtr = std::unique_ptr<boost::dll::shared_library>;

class ModuleManager : public IModuleManager, public cvs::logger::Loggable<ModuleManager> {
 public:
  ModuleManager();
  ~ModuleManager();

  void loadModules(cvs::common::Config&) override;
  void registerTypes(cvs::common::FactoryPtr<std::string>);

  void clear();

 private:
  struct ModuleInfo;
  std::map<std::string, ModuleInfo> modules;
};

}  // namespace cvs::pipeline::impl
