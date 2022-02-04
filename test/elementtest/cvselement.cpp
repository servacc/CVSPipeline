#include <cvs/pipeline/ielement.hpp>
#include <gtest/gtest.h>

CVS_ELEMENT(TestElement,
            "Test element description",
            Fun(int, Arg(int, "Arg 0 description"), Arg(float, "Arg 1 description")),
            Fun(float, Arg(int, "Arg 0 description"))) {
 public:
  int   process(int, float) override { return field; }
  float process(int) override { return field_2; }

 private:
  CVS_FIELD(field, int, "Test field");
  CVS_FIELD(field_2, float, "Another test field");
};

TEST(Element, test) {
  for (auto s : TestElement::describeElement())
    std::cout << s << std::endl;

  std::string config_str = R"(
{
  "field" : 1,
  "field_2" : 0.1
})";

  auto config = cvs::common::CVSConfigBase::load(config_str);

  auto obj = TestElement::makeUPtr(config);
  ASSERT_NE(nullptr, obj);

  EXPECT_EQ(1, obj->process(0, 0.f));
  EXPECT_FLOAT_EQ(0.1f, obj->process(0));
}
