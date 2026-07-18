#include "buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 100

Buffer* buffer_init(void) {
    Buffer* buffer = malloc(sizeof(Buffer));
    if (!buffer) return NULL;
    
    buffer->capacity = INITIAL_CAPACITY;
    buffer->line_count = 1;
    buffer->lines = malloc(sizeof(char*) * buffer->capacity);
    
    buffer->lines[0] = malloc(1);
    buffer->lines[0][0] = '\0';
    
    return buffer;
}

void buffer_free(Buffer* buffer) {
    if (buffer) {
        for (int i = 0; i < buffer->line_count; i++) {
            free(buffer->lines[i]);
        }
        free(buffer->lines);
        free(buffer);
    }
}

void buffer_clear(Buffer* buffer) {
    for (int i = 0; i < buffer->line_count; i++) {
        free(buffer->lines[i]);
    }
    buffer->line_count = 1;
    buffer->lines[0] = malloc(1);
    buffer->lines[0][0] = '\0';
}

void buffer_ensure_capacity(Buffer* buffer, int needed) {
    if (needed >= buffer->capacity) {
        buffer->capacity *= 2;
        buffer->lines = realloc(buffer->lines, sizeof(char*) * buffer->capacity);
    }
}

void buffer_insert_line(Buffer* buffer, int index) {
    if (index < 0 || index > buffer->line_count) return;
    
    buffer_ensure_capacity(buffer, buffer->line_count + 1);
    
    // Décaler les lignes
    for (int i = buffer->line_count - 1; i >= index; i--) {
        buffer->lines[i + 1] = buffer->lines[i];
    }
    
    buffer->lines[index] = malloc(1);
    buffer->lines[index][0] = '\0';
    buffer->line_count++;
}

void buffer_append_line(Buffer* buffer, const char* text) {
    buffer_insert_line(buffer, buffer->line_count);
    buffer->lines[buffer->line_count - 1] = strdup(text);
}

void buffer_delete_line(Buffer* buffer, int index) {
    if (index < 0 || index >= buffer->line_count) return;
    
    free(buffer->lines[index]);
    
    for (int i = index; i < buffer->line_count - 1; i++) {
        buffer->lines[i] = buffer->lines[i + 1];
    }
    
    buffer->line_count--;
}

void buffer_merge_lines(Buffer* buffer, int index1, int index2) {
    if (index1 < 0 || index2 >= buffer->line_count) return;
    
    char* line1 = buffer->lines[index1];
    char* line2 = buffer->lines[index2];
    
    int len1 = strlen(line1);
    int len2 = strlen(line2);
    
    char* merged = malloc(len1 + len2 + 1);
    strcpy(merged, line1);
    strcat(merged, line2);
    
    free(line1);
    buffer->lines[index1] = merged;
    
    buffer_delete_line(buffer, index2);
}

char* buffer_get_line(Buffer* buffer, int index) {
    if (index < 0 || index >= buffer->line_count) return NULL;
    return buffer->lines[index];
}

int buffer_line_length(Buffer* buffer, int index) {
    char* line = buffer_get_line(buffer, index);
    return line ? strlen(line) : 0;
}

int buffer_line_count(Buffer* buffer) {
    return buffer->line_count;
}

void buffer_insert(Buffer* buffer, int line, int col, char c) {
    char* current = buffer_get_line(buffer, line);
    if (!current) return;
    
    int len = strlen(current);
    if (col < 0) col = 0;
    if (col > len) col = len;
    
    char* new_line = malloc(len + 2);
    strncpy(new_line, current, col);
    new_line[col] = c;
    strcpy(new_line + col + 1, current + col);
    
    free(current);
    buffer->lines[line] = new_line;
}

void buffer_insert_text(Buffer* buffer, int line, int col, const char* text, int len) {
    char* current = buffer_get_line(buffer, line);
    if (!current) return;
    
    int current_len = strlen(current);
    if (col < 0) col = 0;
    if (col > current_len) col = current_len;
    
    char* new_line = malloc(current_len + len + 1);
    strncpy(new_line, current, col);
    strncpy(new_line + col, text, len);
    strcpy(new_line + col + len, current + col);
    
    free(current);
    buffer->lines[line] = new_line;
}

void buffer_delete(Buffer* buffer, int line, int col) {
    char* current = buffer_get_line(buffer, line);
    if (!current) return;
    
    int len = strlen(current);
    if (col < 0 || col >= len) return;
    
    char* new_line = malloc(len);
    strncpy(new_line, current, col);
    strcpy(new_line + col, current + col + 1);
    
    free(current);
    buffer->lines[line] = new_line;
}