#include <map>
#include <Arduino.h>
// NOTE:: Needed?
#include <vector>
#include <string>
#include <cstdlib>
#include "parser.h"

enum ParserState {
  PARSER_findTag = 1,   // Looks for tag start ('<')
  PARSER_getTag = 2,    // Checks if tag is either 'path' or 'svg'
  PARSER_findAttr = 3,  // Finds a certain attribute ('viewBox' or 'd')
  PARSER_getAttr = 4,   // Extracts value of attribute
  PARSER_endTag = 5,    // Finds end of tag ('>')
};

// Attributes to look for and extract
const std::string attributes[2] = { "svg.viewBox", "path.d" };

// Parse a valid svg into a class
// Assumes svg is valid and does not check for validity
SVG::SVG(std::string str) {
  // Temporary variables
  std::string buff;                         // Temporary storage
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
                this->viewBox[count] = strtof(buff.substr(0, spaceIndex).c_str(), NULL);
                buff = buff.substr(spaceIndex + 1, -1);
                count++;
              }
              state = PARSER_findTag;
              attr = "";
            } else if (attr == "d") {
              // Add a space before if the path gets
              // added to an already existing path
              if (this->path != "") {
                this->path += " ";
              }
              this->path += buff;
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
}

// Docs: https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths

// Parse path
std::vector<std::pair<char, std::vector<float> > > SVG::followPath() {
  int index = -1;
  std::string curr;
  static std::vector<std::pair<char, std::vector<float> > > actions;

  // TODO: Convert to switch statement
  for (char c: this->path) {
    // Arguments are seperated by spaces
    if (c == ' ' || c == ',') {
      if (!curr.empty()) {
        actions[index].second.push_back(strtof(curr.c_str(), NULL));
        curr = "";
      }

    } else if (isdigit(c) || c == '.' || c == '-') {
      curr += c;

    // A new command is found
    } else {
      // Add the number to the vector if there is one
      if (!curr.empty()) {
        actions[index].second.push_back(strtof(curr.c_str(), NULL));
        curr = "";
      }
      index += 1;
      actions.push_back(
        std::pair<char, std::vector<float> >(c, std::vector<float>(0))
      );
    }    
  }

  // If the string ended, check for analyzed args
  if (!curr.empty()) {
    actions[index].second.push_back(strtof(curr.c_str(), NULL));
    curr = "";
  }

  return actions;
}