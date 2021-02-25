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

  const auto newModule    = lib.get<detail::IModuleConstructor *>("newModule");
  const auto deleteModule = lib.get<detail::IModuleDestructor *>("deleteModule");

  IModuleUPtr result{newModule(), deleteModule};
  if (!result->checkCompatibility()) {
    std::cerr << "Module is incompatible with loaded version of pipeline\n";
    return nullptr;
  }
  return result;
}

}  // namespace cvs::pipeline
