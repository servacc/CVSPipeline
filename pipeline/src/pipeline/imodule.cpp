#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/imodule.hpp>
#include <cvs/pipeline/version.hpp>

namespace cvs::pipeline {

const int IModule::libVersion = cvspipeline_VERSION_MAJOR;

bool IModule::checkCompatibility() const { return libVersion == cvspipeline_VERSION_MAJOR; }

IModuleUPtr makeModule(const boost::dll::shared_library &lib) {
  auto logger = cvs::logger::createLogger("cvs.pipeline.module");
  if (!lib) {
    LOG_ERROR(logger, "Library is not loaded");
    return nullptr;
  }

  if (!lib.has("newModule")) {
    LOG_ERROR(logger, "Module doesn't define newModule");
    return nullptr;
  }
  if (!lib.has("deleteModule")) {
    LOG_ERROR(logger, "Module doesn't define deleteModule");
    return nullptr;
  }

  auto newModule    = &lib.get<detail::IModuleCreator>("newModule");
  auto deleteModule = &lib.get<detail::IModuleDeleter>("deleteModule");

  IModuleUPtr result{newModule(), deleteModule};
  if (!result->checkCompatibility()) {
    LOG_ERROR(logger, "Module is incompatible with loaded version of pipeline");
    return nullptr;
  }
  return result;
}

}  // namespace cvs::pipeline
