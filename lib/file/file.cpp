#include "file.h"

char FileStream::read() {
    return this->file.read();
}

bool FileStream::available() {
    return this->file.available();
}