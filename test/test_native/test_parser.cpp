#include <map>
#include <vector>
#include <string>
#include <unity.h>
#include "parser.h"

void test_parser_valid_svg_default() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
    "</svg>";

  SVG svg = SVG(s);
  float expectedViewBox[4] = { 0.0, 0.0, 500.0, 500.0 };
  TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedViewBox, svg.viewBox, 4);
  TEST_ASSERT_EQUAL_STRING("M 100 100", svg.path.c_str());
}

void test_parser_valid_svg_viewbox_float() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500.98 500.73\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
    "</svg>";

  SVG svg = SVG(s);
  float expectedViewBox[4] = { 0.0, 0.0, 500.98, 500.73 };
  TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedViewBox, svg.viewBox, 4);
}

void test_parser_valid_svg_multiple_path() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
      "<path color=\"red\" d=\"L 100 100\">\n"
    "</svg>";

  SVG svg = SVG(s);
  TEST_ASSERT_EQUAL_STRING("M 100 100 L 100 100", svg.path.c_str());
}

void test_parser_valid_path_default() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
    "</svg>";

  SVG svg = SVG(s);
  std::vector<std::pair<char, std::vector<float> > > res = svg.followPath();
  TEST_ASSERT_EQUAL_FLOAT(100.0, res[0].second[0]);
  TEST_ASSERT_EQUAL_FLOAT(100.0, res[0].second[1]);
}

void test_parser_valid_path_float() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"red\" d=\"L 100.5 100.145\">\n"
    "</svg>";

  SVG svg = SVG(s);
  std::vector<std::pair<char, std::vector<float> > > res = svg.followPath();
  TEST_ASSERT_EQUAL_FLOAT(100.5, res[0].second[0]);
  TEST_ASSERT_EQUAL_FLOAT(100.145, res[0].second[1]);
}

void test_parser_valid_path_no_space() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M100 200\">\n"
    "</svg>";

  SVG svg = SVG(s);
  std::vector<std::pair<char, std::vector<float> > > res = svg.followPath();
  TEST_ASSERT_EQUAL_FLOAT(100.0, res[0].second[0]);
  TEST_ASSERT_EQUAL_FLOAT(100.0, res[0].second[1]);
}

// Run tests on native os
// `pio test -e native -v`
int main() {
  UNITY_BEGIN();
  RUN_TEST(test_parser_valid_svg_default);
  RUN_TEST(test_parser_valid_svg_multiple_path);
  RUN_TEST(test_parser_valid_svg_viewbox_float);
  RUN_TEST(test_parser_valid_path_default);
  UNITY_END();
}