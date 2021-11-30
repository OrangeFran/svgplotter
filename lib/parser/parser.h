#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

class CustomStream {
  public:
    virtual char read() = 0;
    virtual bool available() = 0;
};

class StringStream: public CustomStream {
  private:
    std::string str;

  public:
    StringStream(std::string);
    char read();
    std::string getString();
    bool available();
};

class SVG {
  public:
    CustomStream *stream;
    float viewBox[4];
    float factor; // Factor for scaling 
    // std::string path;
    // std::vector<std::pair<char, std::vector<float> > > actions;
    // float rotation;

    // Constructor
    SVG(CustomStream *);
    bool pathAvailable();
    std::vector<std::pair<char, std::vector<float> > > parseNextPath(void);
    // void setRotation(float);
    void scale(float);
};

float *findViewBox(CustomStream *);
std::string findPath(CustomStream *);

#endif