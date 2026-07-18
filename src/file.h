#ifndef FILE_H
#define FILE_H

#include "buffer.h"

int file_read(const char* filename, Buffer* buffer);
int file_write(const char* filename, Buffer* buffer);
char* file_get_extension(const char* filename);
int file_is_binary(const char* filename);
char* file_get_directory(const char* path);

#endif