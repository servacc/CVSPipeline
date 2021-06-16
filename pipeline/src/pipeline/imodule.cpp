#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/imodule.hpp>
#include <cvs/pipeline/version.hpp>

namespace cvs::pipeline {

const int IModule::libVersion = CVSPipeline_VERSION_MAJOR;

bool IModule::checkCompatibility() const { return libVersion == CVSPipeline_VERSION_MAJOR; }

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

  if (!lib.has("moduleVersionMajor")) {
    LOG_ERROR(logger, "Module doesn't define moduleVersionMajor");
    return nullptr;
  }
  if (!lib.has("moduleVersionMinor")) {
    LOG_ERROR(logger, "Module doesn't define moduleVersionMinor");
    return nullptr;
  }
  if (!lib.has("moduleVersionPatch")) {
    LOG_ERROR(logger, "Module doesn't define moduleVersionPatch");
    return nullptr;
  }

  const auto new_module    = lib.get<detail::IModuleCreator>("newModule")();
  const auto delete_module = &lib.get<detail::IModuleDeleter>("deleteModule");
  const auto version_major = lib.get<detail::IModuleVersion>("moduleVersionMajor")();
  const auto version_minor = lib.get<detail::IModuleVersion>("moduleVersionMinor")();
  const auto version_patch = lib.get<detail::IModuleVersion>("moduleVersionPatch")();

  if (version_major != CVSPipeline_VERSION_MAJOR) {
    LOG_ERROR(logger, "Incompatible version of the CVSPipeline (current {}.{}.{} required {}.{}.{}).",
              CVSPipeline_VERSION_MAJOR, CVSPipeline_VERSION_MINOR, CVSPipeline_VERSION_PATCH, version_major,
              version_minor, version_patch);
    return nullptr;
  }

  LOG_DEBUG(logger, "Current version of the CVSPipeline: {}.{}.{}. Required: {}.{}.{}.", CVSPipeline_VERSION_MAJOR,
            CVSPipeline_VERSION_MINOR, CVSPipeline_VERSION_PATCH, version_major, version_minor, version_patch);

  IModuleUPtr result{new_module, delete_module};
  if (!result->checkCompatibility()) {
    LOG_ERROR(logger, "Module is incompatible with loaded version of pipeline");
    return nullptr;
  }
  return result;
}

}  // namespace cvs::pipeline
