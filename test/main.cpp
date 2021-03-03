#include <boost/dll/runtime_symbol_info.hpp>
#include <cvs/pipeline/imodule.hpp>

#include <iostream>

using namespace cvs::pipeline;

int main() {
  boost::dll::shared_library lib{boost::dll::program_location().parent_path() / "libdummymodule.so",
                                 boost::dll::load_mode::rtld_global | boost::dll::load_mode::rtld_lazy};

  const auto m = cvs::pipeline::makeModule(lib);
  if (!m) {
    std::cerr << "Couldn't load dummy module\n";
    return 1;
  }
  std::cerr << "Loaded module: " << m->name() << '\n';
  return 0;
}
