#ifndef PARSER_H
#define PARSER_H

#include <SD.h>
#include <vector>
#include <string>

class SVG {
  public:
    File file;
    float viewBox[4];
    float factor; // Factor for scaling 
    // std::string path;
    // std::vector<std::pair<char, std::vector<float> > > actions;
    // float rotation;

    // Constructor
    SVG(File);
    bool pathAvailable(void);
    std::vector<std::pair<char, std::vector<float> > > parseNextPath(void);
    // void setRotation(float);
    void scale(float);
};

#endif