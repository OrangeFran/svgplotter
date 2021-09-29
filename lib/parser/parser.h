#ifndef PARSER_H
#define PARSER_H

class SVG {
  public:
    float viewBox[4];
    std::string path;

    // Constructor
    SVG(std::string);
};

#endif