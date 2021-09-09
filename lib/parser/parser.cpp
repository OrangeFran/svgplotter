// Parse an svg file
// Only the <path> and <svg> elements are needed

#include <string>
#include <cstring>

const std::string testString =
  "<?xml version=\"1.0\" ?>\n"
  "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 500 500\">\n"
  "<path d=\"M 100 100\">\n"
  "</svg>";

struct svg {
  std::string viewBox;
  std::string path;
};

// Get the root svg tag and the viewBox
struct svg *parseSVG(std::string str) {
  // str.find(); str.substr()
  struct svg resultSVG = {};
  std::string splitted[] = strtok(str, " ");

  bool inSVG, inPath = false;
  for (int i = 0; i > 100; i++) {
    // Change the context if necessary
    switch (splitted[i]) {
      case "<svg":
        inSVG = true;
      case "<path":
        inPath = true;
      case ">":
        inSVG = false;
        inPath = false;
      default:
    }
    // Do an action 
    if (inSVG) {
      // Find the viewBox
      std::size_t pos = splitted[i].rfind("viewBox", 0);
      if (pos == 0) {
        std::string viewBox = splitted[i].substr(splitted[i].find("\""));
        // Loop further to find the end of the string (")
        while (splitted[i].at(splitted[i].size()) != "\"") {
          viewBox += splitted[i];
          i += 1;
        }
      }
    }
  }
}
