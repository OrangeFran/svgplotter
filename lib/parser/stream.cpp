#include "parser.h"

StringStream::StringStream(std::string s) {
  this->str = s;
}

int StringStream::read() {
  // String empty
  if (this->str.size() == 0) {
    return -1; 
  }
  // Return and remove the first entry
  char c = this->str[0];
  this->str.erase(this->str.begin());
  return (int)c;
}