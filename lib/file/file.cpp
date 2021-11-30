#include <SD.h>
#include "file.h"

FileStream::FileStream(File f) {
    this->file = f;
}

char FileStream::read() {
    return this->file.read();
}

bool FileStream::available() {
    return this->file.available();
}