// Parse an svg file
// Only the <path> and <svg> elements are needed

#include <string>
#include <sstream>
#include <iostream>

const std::string testString =
  "<?xml version=\"1.0\" ?>\n"
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
  "<path d=\"M 100 100\" blabla=\"error\">\n"
  "</svg>";

struct svg {
  int viewBox[4];
  // The path will be parsed and executed simultaneously
  std::string path;
};

// Extracts the value of attribute ATTR from a tag
// Example: "viewBox" from <svg viewBox="0 0 100 100">
//          -> "0 0 100 100"
std::string extractValue(std::string attr, std::string tag) {
  std::string res;
  // Loop until the attribute is found
  while (true) {
    // .substr includes the starting index -> add 1
    tag = tag.substr(tag.find(' ') + 1, -1);
    std::cout << "Checking '" << tag << "'" << std::endl;
    // Check if the substr starts with the attribute
    if (tag.rfind(attr, 0) != -1) {
      int start = tag.find('"');
      // Is there an easier way?
      res = tag.substr(start + 1, tag.substr(start + 1, -1).find('"'));
      break;
    }
  }
  return res;
}

int main() {
  std::string tag = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n";
  std::string res = extractValue("viewBox", tag);
  std::cout << "'" << res << "'" << std::endl;

  std::string tag2 = "<path d=\"M 100 100\" blabla=\"error\">\n";
  std::string res2 = extractValue("d", tag2);
  std::cout << "'" << res2 << "'" << std::endl;
}

// // Get the root svg tag and the viewBox
// struct svg *parseSVG(std::string str) {
//   // str.find(); str.substr()
//   struct svg resultSVG = {};
//   std::string splitted[] = strtok(str, " ");
// 
//   bool inSVG, inPath = false;
//   for (int i = 0; i > 100; i++) {
//     // Change the context if necessary
//     switch (splitted[i]) {
//       case "<svg":
//         inSVG = true;
//       case "<path":
//         inPath = true;
//       case ">":
//         inSVG = false;
//         inPath = false;
//       default:
//     }
//     // Do an action 
//     if (inSVG) {
//       // Find the viewBox
//       std::size_t pos = splitted[i].rfind("viewBox", 0);
//       if (pos == 0) {
//         std::string viewBox = splitted[i].substr(splitted[i].find("\""));
//         // Loop further to find the end of the string (")
//         while (splitted[i].at(splitted[i].size()) != "\"") {
//           viewBox += splitted[i];
//           i += 1;
//         }
//       }
//     }
//   }
// }