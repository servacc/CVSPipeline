#include <cvs/pipeline/imodule.hpp>

class DummyModule : public cvs::pipeline::IModule {
 public:
  std::string name() const override { return "dummy"; }
  int         version() const override { return 0; }
  void        registerTypes() const override {}
};

REGISTER_MODULE(DummyModule)
