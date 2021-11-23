#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

class SVG {
  public:
    float viewBox[4];
    std::string path;
    std::vector<std::pair<char, std::vector<float> > > actions;
    float rotation;

    // Constructor
    SVG(std::string);
    void _followPath(void);
    void setRotation(float);
    void scale(float);
};

#endif