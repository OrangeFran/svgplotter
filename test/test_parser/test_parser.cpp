#include <vector>
#include <string>
#include <unity.h>
#include <stdio.h>

#include "parser.h"

void test_parser_valid_svg_viewbox_float() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500.98 500.73\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  float expectedViewBox[4] = { 0.0, 0.0, 500.98, 500.73 };

  TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedViewBox, svg.viewBox, 4);
}

void test_parser_valid_path_str() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);

  TEST_ASSERT_EQUAL_STRING("M 100 100", findPath(sstream).c_str());
}

void test_parser_valid_multiple_path_str() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
      "<path color=\"red\" d=\"L 200 200\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);

  TEST_ASSERT_EQUAL_STRING("M 100 100", findPath(sstream).c_str());
  TEST_ASSERT_EQUAL_STRING("L 200 200", findPath(sstream).c_str());
}

void test_parser_valid_svg() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  float expectedViewBox[4] = { 0.0, 0.0, 500.0, 500.0 };
  std::vector<std::pair<char, std::vector<float> > > path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_FLOAT_ARRAY(expectedViewBox, svg.viewBox, 4);
  
  TEST_ASSERT_EQUAL_INT((int)'M', (int)path[0].first);
  TEST_ASSERT_EQUAL_FLOAT(100.0, path[0].second[0]);
  TEST_ASSERT_EQUAL_FLOAT(100.0, path[0].second[1]);
}


void test_parser_valid_parse_path() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M 100 100\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  std::vector<std::pair<char, std::vector<float> > > path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_FLOAT(100.0, path[0].second[0]);
  TEST_ASSERT_EQUAL_FLOAT(100.0, path[0].second[1]);
}

void test_parser_valid_parse_path_float() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"red\" d=\"L 100.5 100.145\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  std::vector<std::pair<char, std::vector<float> > > path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_FLOAT(100.5, path[0].second[0]);
  TEST_ASSERT_EQUAL_FLOAT(100.145, path[0].second[1]);
}

void test_parser_valid_parse_path_no_space() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M100 200\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  std::vector<std::pair<char, std::vector<float> > > path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_FLOAT(100.0, path[0].second[0]);
  TEST_ASSERT_EQUAL_FLOAT(200.0, path[0].second[1]);
}

// Run tests on native os
// `pio test -e native -v`
int main() {
  UNITY_BEGIN();
  RUN_TEST(test_parser_valid_svg_viewbox_float);
  RUN_TEST(test_parser_valid_path_str);
  RUN_TEST(test_parser_valid_multiple_path_str);
  RUN_TEST(test_parser_valid_svg);
  RUN_TEST(test_parser_valid_parse_path);
  RUN_TEST(test_parser_valid_parse_path_float);
  RUN_TEST(test_parser_valid_parse_path_no_space);
  UNITY_END();
}

void loop() {}