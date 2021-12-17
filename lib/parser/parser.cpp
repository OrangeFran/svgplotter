#include <map>
#include <vector>
#include <string>
#include <cstdlib>

#include "parser.h"

enum ParserState {
  PARSER_findTag = 1,   // Looks for tag start ('<')
  PARSER_getTag = 2,    // Checks if tag is either 'path' or 'svg'
  PARSER_findAttr = 3,  // Finds a certain attribute ('viewBox' or 'd')
  PARSER_getAttr = 4,   // Extracts value of attribute
  PARSER_endTag = 5     // Finds end of tag ('>')
};

// Attributes to look for and extract
const std::string attributes[2] = { "svg.viewBox", "path.d" };

float *findViewBox(CustomStream *stream) {
  // Temporary variables
  std::string buff;                         // Temporary storage
  std::string attr;                         // Attribute to search for ('viewBox' or 'd' for now)
  enum ParserState state = PARSER_findTag;  // The current state
  bool escaped = false;                     // True if previous character was '\'

  static float viewBox[4];

  char c;
  while (stream->available()) {
    c = stream->read();
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
          if (buff == "svg") {
            state = PARSER_findAttr;
            attr = "viewBox";
            buff = "";
          }
        } else {
          buff += c;
        }
        break;

      case PARSER_endTag:
        if (c == '>') {
          state = PARSER_findTag;
          buff = "";
          return viewBox;
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
            // Convert the buff to a list of ints
            int spaceIndex = 0;
            for (int i = 0; i < 4; i++) {
              spaceIndex = buff.find(' ');
              viewBox[i] = strtof(buff.substr(0, spaceIndex).c_str(), NULL);
              buff = buff.substr(spaceIndex + 1, -1);
            }
            state = PARSER_endTag;
          }
        } else if (c == '\\') {
          escaped = true;
        } else {
          buff += c;
        }
        break;
    }
  }

  return viewBox;
}

std::string findPath(CustomStream *stream) {
  // Temporary variables
  std::string buff;                         // Temporary storage
  std::string attr;                         // Attribute to search for ('viewBox' or 'd' for now)
  enum ParserState state = PARSER_findTag;  // The current state
  bool escaped = false;                     // True if previous character was '\'

  std::string res = "";

  char c;
  while (stream->available()) {
    c = stream->read();
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
          if (buff == "path") {
            state = PARSER_findAttr;
            buff = "";
          } else {
            state = PARSER_endTag;
            buff = "";
          }
        } else {
          buff += c;
        }
        break;

      case PARSER_endTag:
        if (c == '>') {
          if (res != "") {
            return res;
          }
          state = PARSER_findTag;
          buff = "";
        }
        break;

      case PARSER_findAttr:
        if (c == ' ') {
          buff = "";
        // Attribute name ends
        } else if (c == '=') {
          if (buff == "d") {
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
            res = buff;
            state = PARSER_endTag;
            // buff = "";
          }
        } else if (c == '\\') {
          escaped = true;
        } else {
          buff += c;
        }
        break;
    }
  }

  // If the whole file is read and no path was found
  // return EOF to indicate end
  return "";
}

// Parse a valid svg into a class
// Assumes svg is valid and does not check for validity
SVG::SVG(CustomStream *stream) {
  this->stream = stream;
  float *vB = findViewBox(this->stream);
  // Copy array
  for (int i = 0; i < 4; i++) {
    this->viewBox[i] = vB[i];
  }
}

// Docs: https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths

// Parse path
std::vector<std::pair<char, std::vector<float> > > followPath(std::string path) {
  int index = -1;
  std::string curr;

  static std::vector<std::pair<char, std::vector<float> > > actions;

  // Clear the vector
  std::vector<std::pair<char, std::vector<float> > > empty;
  actions = empty;

  for (char c: path) {
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

  // If the string ended, check if command/coordinate still needs
  // to be added to the vector
  if (!curr.empty()) {
    actions[index].second.push_back(strtof(curr.c_str(), NULL));
    curr = "";
  }

  return actions;
}

bool SVG::pathAvailable() {
  return this->stream->available();
}

// Return the next path as a vector of commands and coordinates
// Before calling this function, make sure to see if there are commands left
std::vector<std::pair<char, std::vector<float> > > SVG::parseNextPath() {
  std::string nextPath = findPath(this->stream);
  std::vector<std::pair<char, std::vector<float> > > parsedPath = followPath(nextPath);
  // // Apply scaling
  // if (this->scaleFactor != 0) {
  //   for (int i = 0; i < parsedPath.size(); i++) {
  //     for (int b = 0; b < parsedPath[i].second.size(); b++) {
  //       parsedPath[i].second[b] *= this->scaleFactor;
  //     }
  //   }
  // }
  return parsedPath;
}

void SVG::scale(float width) {
  this->scaleFactor = width/this->viewBox[2];
  // Serial.printf("Factor: %f\n", this->scaleFactor);
  // Update viewBox
  for (int i = 0; i < 4; i++) {
    this->viewBox[i] *= this->scaleFactor;
  }
  // The coordinates get updated when they are read
  // for (int i = 0; i < this->actions.size(); i++) {
  //   // Get the next element
  //   for (int b = 0; b < this->actions[i].second.size(); b++) {
  //     this->actions[i].second[b] *= factor;
  //   }
  // }
}

// // This has to be done when executing the svg
// void SVG::setRotation(float degree) {
//   // // Update the coordinates
//   // for (int i = 0; i < this->actions.size(); i++) {
//   //   // Get the next element
//   //   for (int b = 0; b < this->actions[i].second.size(); b++) {
//   //     this->actions[i].second[b] *= factor;
//   //   }
//   // }
//   this->rotation = degree;
// }