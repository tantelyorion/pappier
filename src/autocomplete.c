#include "autocomplete.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

// Mots-clés par langage
static const char* c_keywords[] = {
    "if", "else", "for", "while", "do", "switch", "case",
    "return", "break", "continue", "goto", "default",
    "int", "char", "float", "double", "void", "long", "short",
    "struct", "enum", "union", "typedef", "sizeof",
    "const", "static", "extern", "register", "volatile",
    "include", "define", "ifdef", "ifndef", "endif",
    "printf", "scanf", "malloc", "calloc", "free",
    NULL
};

static const char* python_keywords[] = {
    "if", "elif", "else", "for", "while", "try", "except",
    "finally", "with", "as", "import", "from", "class",
    "def", "return", "yield", "lambda", "global", "nonlocal",
    "True", "False", "None", "and", "or", "not", "in", "is",
    "print", "len", "range", "open", "self", "super",
    NULL
};

static const char* js_keywords[] = {
    "if", "else", "for", "while", "do", "switch", "case",
    "return", "break", "continue", "throw", "try", "catch",
    "finally", "var", "let", "const", "function", "class",
    "import", "export", "default", "new", "this", "super",
    "console", "log", "error", "warn", "document",
    "window", "alert", "prompt", "confirm",
    NULL
};

Autocomplete* autocomplete_init(void) {
    Autocomplete* ac = malloc(sizeof(Autocomplete));
    ac->suggestions = NULL;
    ac->count = 0;
    ac->selected = 0;
    ac->max_suggestions = 20;
    return ac;
}

void autocomplete_free(Autocomplete* ac) {
    if (ac) {
        if (ac->suggestions) {
            for (int i = 0; i < ac->count; i++) {
                free(ac->suggestions[i]);
            }
            free(ac->suggestions);
        }
        free(ac);
    }
}

static int is_word_char(char c) {
    return isalnum(c) || c == '_';
}

static char* get_current_word(Editor* editor) {
    char* line = buffer_get_line(editor->buffer, editor->cursor_y);
    if (!line) return NULL;
    
    int start = editor->cursor_x;
    while (start > 0 && is_word_char(line[start - 1])) start--;
    
    int len = editor->cursor_x - start;
    if (len == 0) return NULL;
    
    char* word = malloc(len + 1);
    strncpy(word, line + start, len);
    word[len] = '\0';
    return word;
}

static int starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

void autocomplete_update(Editor* editor, Autocomplete* ac) {
    // Nettoyer les anciennes suggestions
    if (ac->suggestions) {
        for (int i = 0; i < ac->count; i++) {
            free(ac->suggestions[i]);
        }
        free(ac->suggestions);
    }
    ac->suggestions = NULL;
    ac->count = 0;
    ac->selected = 0;
    
    char* prefix = get_current_word(editor);
    if (!prefix || strlen(prefix) < 1) {
        if (prefix) free(prefix);
        return;
    }
    
    // Obtenir les mots-clés du langage actuel
    const char** keywords = NULL;
    if (editor->filename) {
        char* ext = strrchr(editor->filename, '.');
        if (ext) {
            if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) {
                keywords = c_keywords;
            } else if (strcmp(ext, ".py") == 0) {
                keywords = python_keywords;
            } else if (strcmp(ext, ".js") == 0) {
                keywords = js_keywords;
            }
        }
    }
    
    if (!keywords) keywords = c_keywords;
    
    // Trouver les suggestions
    ac->suggestions = malloc(sizeof(char*) * ac->max_suggestions);
    
    for (int i = 0; keywords[i] != NULL && ac->count < ac->max_suggestions; i++) {
        if (starts_with(keywords[i], prefix)) {
            ac->suggestions[ac->count] = strdup(keywords[i]);
            ac->count++;
        }
    }
    
    free(prefix);
}

void autocomplete_select_next(Autocomplete* ac) {
    if (ac->count > 0) {
        ac->selected = (ac->selected + 1) % ac->count;
    }
}

void autocomplete_select_prev(Autocomplete* ac) {
    if (ac->count > 0) {
        ac->selected = (ac->selected - 1 + ac->count) % ac->count;
    }
}

void autocomplete_insert(Editor* editor, Autocomplete* ac) {
    if (ac->count == 0) return;
    
    char* word = ac->suggestions[ac->selected];
    if (!word) return;
    
    // Trouver le mot actuel
    char* line = buffer_get_line(editor->buffer, editor->cursor_y);
    int start = editor->cursor_x;
    while (start > 0 && is_word_char(line[start - 1])) start--;
    
    // Supprimer le mot actuel
    for (int i = start; i < editor->cursor_x; i++) {
        buffer_delete(editor->buffer, editor->cursor_y, start);
    }
    editor->cursor_x = start;
    
    // Insérer la suggestion
    for (int i = 0; word[i]; i++) {
        buffer_insert(editor->buffer, editor->cursor_y, editor->cursor_x + i, word[i]);
    }
    editor->cursor_x += strlen(word);
    editor->modified = 1;
}