#ifndef CLIPBOARD_H
#define CLIPBOARD_H

typedef struct {
    char* content;
    int length;
} Clipboard;

Clipboard* clipboard_init(void);
void clipboard_free(Clipboard* clipboard);
void clipboard_set(Clipboard* clipboard, const char* text, int len);
char* clipboard_get(Clipboard* clipboard);
int clipboard_has_content(Clipboard* clipboard);
void clipboard_clear(Clipboard* clipboard);

#endif