#ifndef SNIPPETS_H
#define SNIPPETS_H

#include "editor.h"

typedef struct Snippet {
    char* trigger;          // Mot-clé déclencheur (ex: "for", "if")
    char* description;      // Description
    char* code;             // Code à insérer
    char* language;         // Langage cible (NULL = tous)
    struct Snippet* next;
} Snippet;

typedef struct {
    Snippet* snippets;
    int count;
    char* snippets_dir;
} SnippetManager;

SnippetManager* snippets_init(void);
void snippets_free(SnippetManager* sm);
void snippets_load_defaults(SnippetManager* sm);
void snippets_load_from_file(SnippetManager* sm, const char* filename);
void snippets_save_to_file(SnippetManager* sm, const char* filename);
void snippets_add(SnippetManager* sm, const char* trigger, const char* desc, 
                  const char* code, const char* language);
Snippet* snippets_find(SnippetManager* sm, const char* trigger, const char* language);
void snippets_insert(Editor* editor, Snippet* snippet);
void snippets_render(SnippetManager* sm, int x, int y, int width, int height);

#endif