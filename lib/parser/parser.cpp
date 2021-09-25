// Parse an svg file
// Only the <path> and <svg> elements are needed

#include <string>
#include <cstdlib>
#include <iostream>
#include "parser.h"

// // Extracts the value of attribute ATTR from a tag
// // Example: "viewBox" from <svg viewBox="0 0 100 100">
// //          -> "0 0 100 100"
// std::string extractValue(std::string attr, std::string tag) {
//   std::string res;
//   // Loop until the attribute is found
//   while (true) {
//     // .substr includes the starting index -> add 1
//     tag = tag.substr(tag.find(' ') + 1, -1);
//     // Check if the substr starts with the attribute
//     if (tag.rfind(attr, 0) != -1) {
//       int start = tag.find('"');
//       // Is there an easier way?
//       res = tag.substr(start + 1, tag.substr(start + 1, -1).find('"'));
//       break;
//     }
//   }
//   return res;
// }

enum ParserState {
  PARSER_findTag = 1,   // Looks for tag start ('<')
  PARSER_getTag = 2,    // Checks if tag is either 'path' or 'svg'
  PARSER_findAttr = 3,  // Finds a certain attribute ('viewBox' or 'd')
  PARSER_getAttr = 4,   // Extracts value of attribute
  PARSER_endTag = 5,    // Finds end of tag ('>')
};

// Attributes to look for and extract
const std::string attributes[2] = { "svg.viewBox", "path.d" };

// Parse a valid svg into a struct
// Parser assumes svg is valid and does
// not check for validity
struct SVG parseSVG(std::string str) {
  // Resulting svg
  struct SVG svg;
  
  std::string buff;                       // Temporary storage
  std::string attr;                         // Attribute to search for ('viewBox' or 'd' for now)
  enum ParserState state = PARSER_findTag;  // The current state
  bool escaped = false;                     // True if previous character was '\'

  // Loops through every char of str
  for (char c: str) {
    if (escaped) {
      // Currently extracting value and previous char was '\' 
      // Treat is as a normal character 
      if (state == PARSER_getAttr) {
        buff += c;
        continue;
      }
      escaped = false;
    }

    switch (state) {
      case PARSER_findTag:
        if (c == '<') {
          buff = "";
          state = PARSER_getTag;
        }
        break;
      case PARSER_getTag:
        if (c == ' ') {
          for (int i = 0; i < 2; i++) {
            size_t p = attributes[i].find('.');
            if (buff == attributes[i].substr(0, p)) {
              state = PARSER_findAttr;
              attr = attributes[i].substr(p + 1, -1);
              buff = "";
              break;
            }
          }
          // If we don't need the tag, just look for the end
          if (attr == "") {
            state = PARSER_endTag;
          }
        } else {
          buff += c;
        }
        break;
      case PARSER_endTag:
        if (c == '>') {
          state = PARSER_findTag;
          buff = "";
        }
        break;
      case PARSER_findAttr:
        if (c == ' ') {
          buff = "";
        // Attribute name ends
        } else if (c == '=') {
          if (buff == attr) {
            state = PARSER_getAttr;
            buff = "";
          }
        } else {
          buff += c;
        }
        break;
      case PARSER_getAttr:
        if (c == '"') {
          // End of attribute value
          // TODO: Handle edge case that attribute value is empty -> infinite loop
          if (buff != "") {
            if (attr == "viewBox") {
              // Convert the buff to a list of ints
              int count = 0;
              int spaceIndex = 0;
              while (count < 4) {
                spaceIndex = buff.find(' ');
                svg.viewBox[count] = strtof(buff.substr(0, spaceIndex).c_str(), NULL);
                buff = buff.substr(spaceIndex + 1, -1);
                count++;
              }
              state = PARSER_findTag;
              attr = "";
            } else if (attr == "d") {
              svg.path = buff;
              state = PARSER_findTag;
              attr = "";
            }
            buff = "";
          }
        } else if (c == '\\') {
          escaped = true;
        } else {
          buff += c;
        }
        break;
    }
  }

  return svg;
}

// const std::string testStringValid =
//   "<?xml version=\"1.0\" ?>\n"
//   "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
//     "<path color=\"blue\" d=\"M 100 100\">\n"
//   "</svg>";

// int main() {
//   // // Test 1
//   // std::string tag = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n";
//   // std::string res = extractValue("viewBox", tag);
//   // // "0 0 500 500" is expected
//   // std::cout << "'" << res << "'" << std::endl;
// 
//   // // Test 2
//   // std::string tag2 = "<path d=\"M 100 100\" blabla=\"error\">\n";
//   // std::string res2 = extractValue("d", tag2);
//   // // "M 100 100" is expected
//   // std::cout << "'" << res2 << "'" << std::endl;
// 
//   // Test 1
//   struct SVG svg = parseSVG(testString);
//   std::cout << "svg.viewBox: '" << svg.viewBox << "'" << std::endl;
//   std::cout << "path.d: '" << svg.path << "'" << std::endl;
//   return 0;
// }
