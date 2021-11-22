#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

class SVG {
  public:
    float viewBox[4];
    std::string path;
    std::vector<std::pair<char, std::vector<float> > > actions;

    // Constructor
    SVG(std::string);
    void followPath(void);
    void scale(float);
    void rotate(float);
};

#endif