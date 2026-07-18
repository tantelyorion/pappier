#ifndef EDITOR_H
#define EDITOR_H

#include "buffer.h"
#include "theme.h"

typedef struct {
    Buffer* buffer;
    Theme theme;
    int cursor_x;
    int cursor_y;
    int scroll_x;
    int scroll_y;
    int screen_width;
    int screen_height;
    int modified;
    char* filename;
} Editor;

// Initialisation
Editor* editor_init(void);
void editor_free(Editor* editor);

// Opérations d'édition
void editor_insert_char(Editor* editor, char c);
void editor_delete_char(Editor* editor);
void editor_delete_line(Editor* editor);
void editor_insert_newline(Editor* editor);

// Navigation
void editor_move_cursor(Editor* editor, int dx, int dy);
void editor_move_to_line(Editor* editor, int line);
void editor_move_to_column(Editor* editor, int col);
void editor_move_word(Editor* editor, int direction);

// Fichier
int editor_load_file(Editor* editor, const char* filename);
int editor_save_file(Editor* editor, const char* filename);

// Affichage
void editor_render(Editor* editor);
void editor_status_bar(Editor* editor);

// Clipbard
void editor_copy(Editor* editor);
void editor_cut(Editor* editor);
void editor_paste(Editor* editor);

// Recherche
void editor_find(Editor* editor, const char* query);
void editor_find_next(Editor* editor);
void editor_replace(Editor* editor, const char* find, const char* replace);

#endif