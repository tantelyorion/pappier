#ifndef BUFFER_H
#define BUFFER_H

typedef struct Buffer {
    char** lines;
    int line_count;
    int capacity;
} Buffer;

Buffer* buffer_init(void);
void buffer_free(Buffer* buffer);
void buffer_clear(Buffer* buffer);

void buffer_insert_line(Buffer* buffer, int index);
void buffer_append_line(Buffer* buffer, const char* text);
void buffer_delete_line(Buffer* buffer, int index);
void buffer_merge_lines(Buffer* buffer, int index1, int index2);

char* buffer_get_line(Buffer* buffer, int index);
int buffer_line_length(Buffer* buffer, int index);
int buffer_line_count(Buffer* buffer);

void buffer_insert(Buffer* buffer, int line, int col, char c);
void buffer_insert_text(Buffer* buffer, int line, int col, const char* text, int len);
void buffer_delete(Buffer* buffer, int line, int col);

#endif