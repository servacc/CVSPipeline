#include <iostream>

#include <cvs/config/config_base.hpp>

#include "gtest/gtest.h"

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

constexpr double TEST_JSON_REQUIRED_INNER_VALUE = 12415.123123;
constexpr auto TEST_JSON_REQUIRED_INNER_HASH = "hHHhshAHSAs0-i0 1i2=uq9f jf3";
constexpr double TEST_JSON_REQUIRED_DISTANCE = 7.2;
constexpr float TEST_JSON_LENGTH = 7.2;
constexpr float TEST_JSON_VALUE = -0.0001;

const std::string parsing_test_json =
  "{ \n"
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

Config_object(Parsing_test_config,
  Object(required,
    Value(distance, std::remove_cv<decltype(TEST_JSON_REQUIRED_DISTANCE)>::type),
    Value_optional(call, std::string),
    Object(inner,
      Value(value, std::remove_cv<decltype(TEST_JSON_REQUIRED_INNER_VALUE)>::type),
      Value(hash, std::remove_cv<decltype(std::string(TEST_JSON_REQUIRED_INNER_HASH))>::type)
    )
  ),
  Object_optional(optional,
    Value(distance, double),
    Value_optional(hash, std::remove_cv<decltype(std::string(TEST_JSON_REQUIRED_INNER_HASH))>::type)
  ),
  Value_default(length, std::remove_cv<decltype(TEST_JSON_LENGTH)>::type, TEST_JSON_LENGTH),
  Value(value, std::remove_cv<decltype(TEST_JSON_VALUE)>::type),
  Value_optional(global, Config)
)

TEST(parsing_test, main_test) {
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

  auto test_result = Parsing_test_config::parse_and_make(root);

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