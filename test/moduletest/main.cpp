#include <boost/dll/runtime_symbol_info.hpp>
#include <cvs/logger/logging.hpp>
#include <cvs/pipeline/imodule.hpp>

#include <iostream>

using namespace cvs::pipeline;

int main() {
  cvs::logger::initLoggers();

  boost::dll::shared_library lib{boost::filesystem::path(TEST_MODULES_PATH) / "libdummymodule.so",
                                 boost::dll::load_mode::rtld_global | boost::dll::load_mode::rtld_lazy};

  const auto m = cvs::pipeline::makeModule(lib);
  if (!m) {
    std::cerr << "Couldn't load dummy module\n";
    return 1;
  }
  std::cerr << "Loaded module: " << m->name() << '\n';
  return 0;
}
