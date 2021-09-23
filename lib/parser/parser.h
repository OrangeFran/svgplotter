struct SVG {
  std::string viewBox;
  // The paths will be parsed and executed simultaneously
  std::string path;
};

struct SVG parseSVG(std::string);