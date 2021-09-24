#include <string>
#include <unity.h>
#include "parser.h"

const std::string testStringValid =
  "<?xml version=\"1.0\" ?>\n"
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
    "<path color=\"blue\" d=\"M 100 100\">\n"
  "</svg>";

void test_valid() {
  struct SVG svg = parseSVG(testStringValid);
  TEST_ASSERT_EQUAL_STRING("0 0 500 500", svg.viewBox.c_str());
  TEST_ASSERT_EQUAL_STRING("M 100 100", svg.path.c_str());
}

// Run tests on native os
// ```pio test -e native -v```
int main() {
  UNITY_BEGIN();
  RUN_TEST(test_valid);
  UNITY_END();
}