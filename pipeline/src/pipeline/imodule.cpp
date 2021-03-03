#include <cvs/pipeline/imodule.hpp>

namespace cvs::pipeline {

const int IModule::libVersion = IModule::kVersion;

IModuleUPtr makeModule(const boost::dll::shared_library &lib) {
  if (!lib) {
    std::cerr << "Library is not loaded\n";
    return nullptr;
  }

  if (!lib.has("newModule")) {
    std::cerr << "Module doesn't define newModule\n";
    return nullptr;
  }
  if (!lib.has("deleteModule")) {
    std::cerr << "Module doesn't define deleteModule\n";
    return nullptr;
  }

  auto newModule    = &lib.get<detail::IModuleCreator>("newModule");
  auto deleteModule = &lib.get<detail::IModuleDeleter>("deleteModule");

  IModuleUPtr result{newModule(), deleteModule};
  if (!result->checkCompatibility()) {
    std::cerr << "Module is incompatible with loaded version of pipeline\n";
    return nullptr;
  }
  return result;
}

}  // namespace cvs::pipeline
