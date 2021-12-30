#include <map>
#include <vector>
#include <string>
#include <cstdlib>
#include <cmath>

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

  float *viewBox = (float *)malloc(4 * sizeof(float));

  char c;
  while (true) {
    c = stream->read();
    // Check if all characters are read
    if (c == -1) {
      break;
    }

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

  int i;
  char c;
  while (true) {
    i = stream->read();
    // Check if all characters are read
    if (i == -1) {
      break;
    }

    c = (char)i;
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
            // Return result
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
  this->scaleFactor = 0;
  this->rotation = 0;
  float *vB = findViewBox(this->stream);
  // Copy array
  for (int i = 0; i < 4; i++) {
    this->viewBox[i] = vB[i];
  }
}

// Docs: https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths

// Parse path
std::vector<std::pair<char, std::vector<float> > > *followPath(std::string path) {
  int index = -1;
  std::string curr;
  auto *actions =
    new std::vector<std::pair<char, std::vector<float> > >;

  for (char c: path) {
    // Arguments are seperated by spaces
    if (c == ' ' || c == ',') {
      if (!curr.empty()) {
        actions->at(index).second.push_back(strtof(curr.c_str(), NULL));
        curr = "";
      }

    } else if (isdigit(c) || c == '.' || c == '-') {
      curr += c;

    // A new command is found
    } else {
      // Add the number to the vector if there is one
      if (!curr.empty()) {
        actions->at(index).second.push_back(strtof(curr.c_str(), NULL));
        curr = "";
      }
      index += 1;
      actions->push_back(
        std::pair<char, std::vector<float> >(c, std::vector<float>(0))
      );
    }    
  }

  // If the string ended, check if command/coordinate still needs
  // to be added to the vector
  if (!curr.empty()) {
    actions->at(index).second.push_back(strtof(curr.c_str(), NULL));
    curr = "";
  }

  return actions->size() == 0 ? NULL : actions;
}

void SVG::scaleAndRotatePath(std::vector<std::pair<char, std::vector<float> > > *path) {
}

// Return the next path as a vector of commands and coordinates
// Before calling this function, make sure to see if there are commands left
std::vector<std::pair<char, std::vector<float> > > *SVG::parseNextPath() {
  std::string nextPath = findPath(this->stream);
  auto parsedPath = followPath(nextPath);

  if (parsedPath != NULL) {

    float position[2] = {0, 0};
    // Save values for faster access
    float sin_r = sin(-this->rotation);
    float cos_r = cos(-this->rotation);
    std::vector<float> c_cpy;

    for (auto c = parsedPath->begin(); c != parsedPath->end(); c++) {

      if (this->scaleFactor != 0) {
        for (int b = 0; b < c->second.size(); b++) {
          c->second[b] *= this->scaleFactor;
        }
      }

      // Convert H and V to Ls
      // Convert relative to absolute
      switch (c->first) {
        // Commands with x and y coordinates 
        case 'm':
        case 'l':
        case 'q':
        case 't':
        case 'c':
        case 's':
        case 'z':
          for (int i = 0; i < c->second.size() / 2; i++) {
            c->second[i * 2] += position[0];
            c->second[i * 2 + 1] += position[1];
          }
          c->first = toupper(c->first);
          break;

        // Commands with x or y coordinate
        // Need to be converted to L commands
        case 'v':
          c->second[0] += position[1];
        case 'V':
          c->first = 'L';
          c->second.insert(c->second.begin(), position[0]);
          break;

        case 'h':
          c->second[0] += position[0];
        case 'H':
          c->first = 'L';
          c->second.push_back(position[1]);
          break;

        default:
          break;
      }
      

      // Make a copy for access to old values after modifying 
      c_cpy = c->second;

      int vec_size = c->second.size();

      // The last to coordinates resemble the new position
      position[0] = c_cpy[vec_size - 2];
      position[1] = c_cpy[vec_size - 1];

      // Rotation formulae (flipped axes) (clockwise):
      // https://en.wikipedia.org/wiki/Rotation_matrix#Non-standard_orientation_of_the_coordinate_system
      //   -> x = cos(..) * x - sin(..) * y
      //   -> y = sin(..) * x + cos(..) * y
      if (this->rotation != 0) {
        for (int i = 0; i < vec_size / 2; i++) {
          c->second[i * 2] = cos_r * c_cpy[i * 2] - sin_r * c_cpy[i * 2 + 1];
          c->second[i * 2 + 1] = sin_r * c_cpy[i * 2] + cos_r * c_cpy[i * 2 + 1];
        }
      }

      // switch(c->first) {
      //   case 'M':
      //   case 'L':
      //   case 'T':
      //     position[0] = c->second[0];
      //     position[1] = c->second[1];
      //     if (this->rotation != 0) {
      //       c->second[0] = sin_r * c_cpy[0] + cos_r * c_cpy[1];
      //       c->second[1] = cos_r * c_cpy[0] - sin_r * c_cpy[1];
      //     }
      //     break;

      //   case 'Q':
      //   case 'S':
      //     position[0] = c->second[2];
      //     position[1] = c->second[3];
      //     if (this->rotation != 0) {
      //       for (int i = 0; i < 2; i++) {
      //         c->second[i * 2] = sin_r * c_cpy[i * 2] + cos_r * c_cpy[i * 2 + 1];
      //         c->second[i * 2 + 1] = cos_r * c_cpy[i * 2] - sin_r * c_cpy[i * 2 + 1];
      //       }
      //     }
      //     break;

      //   case 'C':
      //     position[0] = c->second[4];
      //     position[1] = c->second[5];
      //     if (this->rotation != 0) {
      //       for (int i = 0; i < 3; i++) {
      //         c->second[i * 2] = sin_r * c_cpy[i * 2] + cos_r * c_cpy[i * 2 + 1];
      //         c->second[i * 2 + 1] = cos_r * c_cpy[i * 2] - sin_r * c_cpy[i * 2 + 1];
      //       }
      //     }
      //     break;

      //   default:
      //     break;
      // }
    }
  }

  return parsedPath;
}

void SVG::setScaleFactor(float factor) {
  this->scaleFactor = factor;
  // Update viewBox
  for (int i = 0; i < 4; i++) {
    this->viewBox[i] *= this->scaleFactor;
  }
}

void SVG::setRotation(float degree) {
  this->rotation = degree;
}