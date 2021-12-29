#define _USE_MATH_DEFINES

#include <vector>
#include <string>
#include <math.h>
#include <unity.h>
#include <stdio.h>

#include "parser.h"
#include "test_transformations.h"

void test_parser_valid_parse_path_with_scale() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M150 400\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  svg.setScaleFactor(100.0/svg.viewBox[2]);
  auto *path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_FLOAT(30.0, path->at(0).second[0]);
  TEST_ASSERT_EQUAL_FLOAT(80.0, path->at(0).second[1]);
}

void test_parser_valid_parse_path_relative_to_absolute() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M10 10 l 10 10 q -10 -30 44.5 6\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  auto *path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_FLOAT(20.0, path->at(1).second[0]);
  TEST_ASSERT_EQUAL_FLOAT(20.0, path->at(1).second[1]);
  TEST_ASSERT_EQUAL_FLOAT(10.0, path->at(2).second[0]);
  TEST_ASSERT_EQUAL_FLOAT(-10.0, path->at(2).second[1]);
  TEST_ASSERT_EQUAL_FLOAT(64.5, path->at(2).second[2]);
  TEST_ASSERT_EQUAL_FLOAT(26.0, path->at(2).second[3]);
}

void test_parser_valid_parse_path_with_rotation() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M10 10\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  svg.setRotation(M_PI / 2.0);
  auto *path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_FLOAT(10.0, path->at(0).second[0]);
  TEST_ASSERT_EQUAL_FLOAT(-10.0, path->at(0).second[1]);
}