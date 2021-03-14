#include <iostream>

#include <cvs/config/config_base.hpp>

#include "gtest/gtest.h"


constexpr double TEST_JSON_REQUIRED_INNER_VALUE = 12415.123123;
constexpr auto TEST_JSON_REQUIRED_INNER_HASH = "hHHhshAHSAs0-i0 1i2=uq9f jf3";
constexpr double TEST_JSON_REQUIRED_DISTANCE = 7.2;
constexpr float TEST_JSON_LENGTH = 7.2;
constexpr float TEST_JSON_VALUE = -0.0001;

const std::string parsing_test_json =
  "{ \n"
    "\"moduleName\": \"test_module\", \n"
    "\"required\": { \n"
      "\"inner\": { \n"
        "\"value\": \"" + std::to_string(TEST_JSON_REQUIRED_INNER_VALUE) + "\",\n"
        "\"hash\": \"" + std::string(TEST_JSON_REQUIRED_INNER_HASH) + "\"\n"
      "},\n"
      "\"distance\": \"" + std::to_string(TEST_JSON_REQUIRED_DISTANCE) + "\"\n"
      //"\"call\": \"Wow\"\n"
    "}, \n"
    "\"optional\": {\n"
      "\"hash\": \"" + std::string(TEST_JSON_REQUIRED_INNER_HASH) + "\"\n"
    "},\n"
  //"\"length\": \"15\", "
    "\"value\": \"" + std::to_string(TEST_JSON_VALUE) + "\"\n"
  "}";

DECLARE_CONFIG( ParsingTestConfig,
  OBJECT( required,
    VALUE( distance, std::remove_cv<decltype(TEST_JSON_REQUIRED_DISTANCE)>::type),
    VALUE_OPTIONAL( call, std::string),
    OBJECT( inner,
      VALUE( value, std::remove_cv<decltype(TEST_JSON_REQUIRED_INNER_VALUE)>::type),
      VALUE( hash, std::remove_cv<decltype(std::string(TEST_JSON_REQUIRED_INNER_HASH))>::type)
    )
  ),
  OBJECT_OPTIONAL( optional,
    VALUE( distance, double, true),
    VALUE_OPTIONAL( hash, std::remove_cv<decltype(std::string(TEST_JSON_REQUIRED_INNER_HASH))>::type)
  ),
  VALUE_DEFAULT( length, std::remove_cv<decltype(TEST_JSON_LENGTH)>::type, TEST_JSON_LENGTH),
  VALUE( value, std::remove_cv<decltype(TEST_JSON_VALUE)>::type),
  VALUE_OPTIONAL( global, Config)
)

TEST(main_test, parsing_test) {
  std::stringstream ss;
  ss << parsing_test_json;
  boost::property_tree::ptree root;

  try {
    boost::property_tree::read_json(ss, root);
  }
  catch (const boost::property_tree::json_parser::json_parser_error& exception) {
    FAIL() << "Unexpected json_parser_error: " << exception.what() << ", in line number: " << exception.line();
  }
  catch (const std::exception& exception) {
    FAIL() << "Unexpected std::exception: " << exception.what();
  }
  catch (...) {
    FAIL() << "Unexpected unknown exception";
  }

  auto test_result = ParsingTestConfig::make(root);

  ASSERT_TRUE(test_result.has_value()) << "Parsing failed";
  EXPECT_EQ(test_result->_required._inner._value, TEST_JSON_REQUIRED_INNER_VALUE) << "required->inner->value parse failed";
  EXPECT_EQ(test_result->_required._inner._hash, TEST_JSON_REQUIRED_INNER_HASH) << "required->inner->hash parse failed";
  EXPECT_EQ(test_result->_required._distance, TEST_JSON_REQUIRED_DISTANCE) << "required->distance parse failed";
  EXPECT_EQ(test_result->_required._call, std::nullopt) << "required->call parse failed";
  EXPECT_EQ(test_result->_optional, std::nullopt) << "required->optional parse failed";
  EXPECT_EQ(test_result->_length, TEST_JSON_LENGTH) << "required->length parse failed";
  EXPECT_EQ(test_result->_value, TEST_JSON_VALUE) << "required->value parse failed";
  EXPECT_EQ(test_result->_global, std::nullopt) << "required->global parse failed";
}

TEST(main_test, module_config_test) {
  const double global_distance = 999.003;
  const std::string module_name = "test_module";

  const std::string module_config_test_json =
    "{ \n"
      "\"" + module_name + "\": " + parsing_test_json + ","
      "\"global\": {\n"
        "\"hash\": \"" + std::string(TEST_JSON_REQUIRED_INNER_HASH) + "\",\n"
        "\"value\": \"" + std::to_string(TEST_JSON_REQUIRED_INNER_VALUE) + "\",\n"
        "\"distance\": \"" + std::to_string(global_distance) + "\",\n"
        "\"some\": \"ololo\"\n"
      "}\n"
    "}";


  std::stringstream ss;
  ss << module_config_test_json;
  boost::property_tree::ptree root;

  try {
    boost::property_tree::read_json(ss, root);
  }
  catch (const boost::property_tree::json_parser::json_parser_error& exception) {
    FAIL() << "Unexpected json_parser_error: " << exception.what() << ", in line number: " << exception.line();
  }
  catch (const std::exception& exception) {
    FAIL() << "Unexpected std::exception: " << exception.what();
  }
  catch (...) {
    FAIL() << "Unexpected unknown exception";
  }

  // if "auto global" only link will be returned, then broken after erasing
  std::optional<boost::property_tree::ptree> global = Utils::boostOptionalToStd(root.get_child_optional("global"));

  ASSERT_TRUE(global.has_value()) << "Parsing failed";

  if (global) {
    root.erase("global");
  }

  {
    auto test_global = root.get_child_optional("global");
    ASSERT_FALSE(test_global.has_value()) << "Erasing failed";
  }

  std::vector<Config> modules_configs;
  for (const auto& element : root) {
    modules_configs.emplace_back(element.second, global, element.first);
  }

  EXPECT_EQ(modules_configs.size(), 1) << "There must be 1 parsed config";
  auto& test_module = modules_configs[0];
  EXPECT_EQ(test_module.getName(), module_name) << "Wrong module name";
  EXPECT_EQ(
    test_module.getValueOptional<std::remove_cv<decltype(TEST_JSON_VALUE)>::type>("value"),
    TEST_JSON_VALUE
  ) << "\'value\' parsing failed";

  auto test_module_result = test_module.parse<ParsingTestConfig>();
  ASSERT_TRUE(test_module_result.has_value()) << "Test module parsing failed";
  ASSERT_TRUE(test_module_result->_optional.has_value()) << "required->optional parse failed";
  EXPECT_EQ(test_module_result->_optional.value()._distance, global_distance) << "global_distance parse failed";
}