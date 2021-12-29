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

  TEST_ASSERT_EQUAL_INT((int)'L', (int)path->at(1).first);
  TEST_ASSERT_EQUAL_INT((int)'Q', (int)path->at(2).first);
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
  float degree = M_PI / 4.0;
  svg.setRotation(degree);
  auto *path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_FLOAT(sin(degree) * 10 + cos(degree) * 10, path->at(0).second[0]);
  TEST_ASSERT_EQUAL_FLOAT(cos(degree) * 10 - sin(degree) * 10, path->at(0).second[1]);
}

void test_parser_valid_parse_path_with_rotation_scale_and_to_absolute() {
  const std::string s =
    // "<?xml version=\"1.0\" ?>\n"
    "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
      "<path color=\"blue\" d=\"M100 100 l 100 400\">\n"
    "</svg>";

  CustomStream *sstream = new StringStream(s);
  SVG svg = SVG(sstream);
  float degree = M_PI / 4.0;
  svg.setRotation(degree);
  svg.setScaleFactor(0.5);
  auto *path = svg.parseNextPath();

  TEST_ASSERT_EQUAL_CHAR((int)'L', (int)path->at(1).first);
  TEST_ASSERT_EQUAL_FLOAT((sin(degree) * 100 + cos(degree) * 100) * 0.5, path->at(0).second[0]);
  TEST_ASSERT_EQUAL_FLOAT((cos(degree) * 100 - sin(degree) * 100) * 0.5, path->at(0).second[1]);
  TEST_ASSERT_EQUAL_FLOAT((sin(degree) * 200 + cos(degree) * 500) * 0.5, path->at(1).second[0]);
  TEST_ASSERT_EQUAL_FLOAT((cos(degree) * 200 - sin(degree) * 500) * 0.5, path->at(1).second[1]);
}