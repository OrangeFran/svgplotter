#include <string>
#include <unity.h>
#include "parser.h"

const std::string testStringValidDefault =
  "<?xml version=\"1.0\" ?>\n"
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
    "<path color=\"blue\" d=\"M 100 100\">\n"
  "</svg>";

const std::string testStringValidFloat =
  "<?xml version=\"1.0\" ?>\n"
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500.98 500.73\">\n"
    "<path color=\"blue\" d=\"M 100 100\">\n"
  "</svg>";

const std::string testStringValidDoublePath =
  "<?xml version=\"1.0\" ?>\n"
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
    "<path color=\"blue\" d=\"M 100 100\">\n"
    "<path color=\"red\" d=\"L 100 100\">\n"
  "</svg>";

void test_valid_default() {
  struct SVG svg = parseSVG(testStringValidDefault);
  float expectedViewBox[4] = {0.0, 0.0, 500.0, 500.0};
  TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedViewBox, svg.viewBox, 4);
  TEST_ASSERT_EQUAL_STRING("M 100 100", svg.path.c_str());
}

void test_valid_float() {
  struct SVG svg = parseSVG(testStringValidFloat);
  float expectedViewBox[4] = {0.0, 0.0, 500.98, 500.73};
  TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedViewBox, svg.viewBox, 4);
}

void test_valid_double_path() {
  struct SVG svg = parseSVG(testStringValidDoublePath);
  TEST_ASSERT_EQUAL_STRING("M 100 100 L 100 100", svg.path.c_str());
}

// Run tests on native os
// ```pio test -e native -v```
int main() {
  UNITY_BEGIN();
  RUN_TEST(test_valid_default);
  RUN_TEST(test_valid_float);
  RUN_TEST(test_valid_double_path);
  UNITY_END();
}