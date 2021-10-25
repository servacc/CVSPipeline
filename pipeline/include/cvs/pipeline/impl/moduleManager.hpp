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

  void clear();

 private:
  struct ModuleInfo;
  std::map<std::string, ModuleInfo> modules;

  std::set<std::filesystem::path> module_path;
};

}  // namespace cvs::pipeline::impl
