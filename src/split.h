#ifndef SPLIT_H
#define SPLIT_H

#include "editor.h"

typedef enum {
    SPLIT_VERTICAL,
    SPLIT_HORIZONTAL
} SplitType;

typedef struct SplitPane {
    Editor* editor;
    struct SplitPane* left;
    struct SplitPane* right;
    struct SplitPane* parent;
    SplitType type;
    float ratio;
    int active;
} SplitPane;

SplitPane* split_create(Editor* editor);
void split_free(SplitPane* pane);
void split_vertical(SplitPane* pane);
void split_horizontal(SplitPane* pane);
void split_close(SplitPane* pane, SplitPane* target);
void split_focus_next(SplitPane* pane);
void split_focus_prev(SplitPane* pane);
void split_resize(SplitPane* pane, float ratio);
void split_render_all(SplitPane* pane, int x, int y, int width, int height);
SplitPane* split_get_active(SplitPane* pane);

#endif