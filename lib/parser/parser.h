struct SVG {
  float viewBox[4];
  // The paths will be parsed and executed simultaneously
  std::string path;
};

struct SVG parseSVG(std::string);