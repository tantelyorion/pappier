#include "clipboard.h"
#include <stdlib.h>
#include <string.h>

Clipboard* clipboard_init(void) {
    Clipboard* cb = malloc(sizeof(Clipboard));
    cb->content = NULL;
    cb->length = 0;
    return cb;
}

void clipboard_free(Clipboard* clipboard) {
    if (clipboard) {
        if (clipboard->content) free(clipboard->content);
        free(clipboard);
    }
}

void clipboard_set(Clipboard* clipboard, const char* text, int len) {
    if (clipboard->content) {
        free(clipboard->content);
    }
    
    clipboard->content = malloc(len + 1);
    memcpy(clipboard->content, text, len);
    clipboard->content[len] = '\0';
    clipboard->length = len;
}

char* clipboard_get(Clipboard* clipboard) {
    return clipboard->content;
}

int clipboard_has_content(Clipboard* clipboard) {
    return clipboard->content != NULL && clipboard->length > 0;
}

void clipboard_clear(Clipboard* clipboard) {
    if (clipboard->content) {
        free(clipboard->content);
        clipboard->content = NULL;
        clipboard->length = 0;
    }
}