#pragma once

#include <cvs/common/factory.hpp>
#include <cvs/logger/loggable.hpp>
#include <cvs/pipeline/imodule.hpp>
#include <cvs/pipeline/imoduleManager.hpp>

#include <filesystem>
#include <map>
#include <set>

namespace cvs::pipeline::impl {

class ModuleManager : public IModuleManager, public cvs::logger::Loggable<ModuleManager> {
 public:
  static std::unique_ptr<ModuleManager> make(const common::Properties&);

  ModuleManager();
  ~ModuleManager();

  void loadModules() override;
  void registerTypes(cvs::common::FactoryPtr<std::string>) override;

  std::string loadModule(const std::filesystem::path&);

  std::vector<std::string> loadedModules() const;

  void clear();

  const std::set<std::filesystem::path>& modulesPaths() const;

  std::vector<std::string> loadModulesFrom(const std::filesystem::path&);
  void                     registerTypesFrom(const std::string&, cvs::common::FactoryPtr<std::string>);

  std::string moduleName(const std::string& name) const;
  int         moduleVersion(const std::string& name) const;

 private:
  struct ModuleInfo;
  std::map<std::string, ModuleInfo> modules;

  std::set<std::filesystem::path> module_path;
};

}  // namespace cvs::pipeline::impl
