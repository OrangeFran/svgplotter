#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <vector>

class SVG {
  public:
    float viewBox[4];
    std::string path;

    // Constructor
    SVG(std::string);
    std::map<char, std::vector<float> > followPath(void);
};

#endif