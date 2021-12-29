#include <unity.h>
#include <stdio.h>

#include "test_parsing/test_parsing.h"
#include "test_transformations/test_transformations.h"

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
  RUN_TEST(test_parser_valid_parse_path_with_scale);
  RUN_TEST(test_parser_valid_parse_path_with_rotation);
  RUN_TEST(test_parser_valid_parse_path_relative_to_absolute);
  UNITY_END();
}

void loop() {}