#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

// A custom class to allow the parser
// to accept either a file or a string through
// this custom stream class
// Reason: the parser can be tested on the native
// platform without requiring the File object from the
// Arduino library
class CustomStream {
  public:
    // Returns the next character
    virtual int read() = 0;
};

class StringStream: public CustomStream {
  private:
    std::string str;

  public:
    StringStream(std::string);
    std::string getString();
    int read();
};

class SVG {
  public:
    CustomStream *stream;
    float viewBox[4];
    float scaleFactor;
    float rotation;

    // Constructor
    SVG(CustomStream *);
    bool pathAvailable();
    // Returns the next path
    std::vector<std::pair<char, std::vector<float> > > *parseNextPath(void);
    void setScaleFactor(float);
    void setRotation(float);
    // Apply them
    void scaleAndRotatePath(std::vector<std::pair<char, std::vector<float> > > *);
};

float *findViewBox(CustomStream *);
std::string findPath(CustomStream *);

#endif