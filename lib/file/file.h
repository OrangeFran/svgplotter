#ifndef FILE_H
#define FILE_H

#include <SD.h>
#include "parser.h"

class FileStream: public CustomStream {
  private:
    File file;

  public:
    FileStream(File);
    char read();
};

#endif