#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int file_read(const char* filename, Buffer* buffer) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    
    buffer_clear(buffer);
    
    char line[4096];
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        buffer_append_line(buffer, line);
    }
    
    fclose(file);
    return 0;
}

int file_write(const char* filename, Buffer* buffer) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    
    for (int i = 0; i < buffer_line_count(buffer); i++) {
        char* line = buffer_get_line(buffer, i);
        fprintf(file, "%s\n", line);
    }
    
    fclose(file);
    return 0;
}

char* file_get_extension(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (dot) {
        return strdup(dot + 1);
    }
    return NULL;
}

int file_is_binary(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return 0;
    
    unsigned char buffer[512];
    size_t read = fread(buffer, 1, 512, file);
    fclose(file);
    
    for (size_t i = 0; i < read; i++) {
        if (buffer[i] == 0x00) return 1;
    }
    
    return 0;
}

char* file_get_directory(const char* path) {
    char* dir = strdup(path);
    char* last_slash = strrchr(dir, '/');
    if (last_slash) {
        *last_slash = '\0';
    } else {
        free(dir);
        return strdup(".");
    }
    return dir;
}