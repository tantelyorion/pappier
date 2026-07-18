#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include "editor.h"

typedef struct {
    char** suggestions;
    int count;
    int selected;
    int max_suggestions;
} Autocomplete;

Autocomplete* autocomplete_init(void);
void autocomplete_free(Autocomplete* ac);
void autocomplete_update(Editor* editor, Autocomplete* ac);
void autocomplete_select_next(Autocomplete* ac);
void autocomplete_select_prev(Autocomplete* ac);
void autocomplete_insert(Editor* editor, Autocomplete* ac);

// Mots-clés par langage
void autocomplete_load_keywords(Autocomplete* ac, const char* extension);

#endif