#ifndef PARSER_H
#define PARSER_H

#include <vector>

class SVG {
  public:
    float viewBox[4];
    std::string path;

    // Constructor
    SVG(std::string);
    std::vector<std::pair<char, std::vector<float> > > followPath(void);
};

#endif