#include "snippets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

SnippetManager* snippets_init(void) {
    SnippetManager* sm = malloc(sizeof(SnippetManager));
    sm->snippets = NULL;
    sm->count = 0;
    sm->snippets_dir = strdup("~/.pappier/snippets");
    
    snippets_load_defaults(sm);
    return sm;
}

void snippets_free(SnippetManager* sm) {
    if (!sm) return;
    
    Snippet* current = sm->snippets;
    while (current) {
        Snippet* next = current->next;
        if (current->trigger) free(current->trigger);
        if (current->description) free(current->description);
        if (current->code) free(current->code);
        if (current->language) free(current->language);
        free(current);
        current = next;
    }
    
    if (sm->snippets_dir) free(sm->snippets_dir);
    free(sm);
}

void snippets_load_defaults(SnippetManager* sm) {
    // C/C++ Snippets
    snippets_add(sm, "for", "Boucle for", 
        "for (int i = 0; i < n; i++) {\n    \n}", "c");
    snippets_add(sm, "for", "Boucle for", 
        "for (int i = 0; i < n; i++) {\n    \n}", "cpp");
    
    snippets_add(sm, "if", "Condition if", 
        "if (condition) {\n    \n}", "c");
    snippets_add(sm, "if", "Condition if", 
        "if (condition) {\n    \n}", "cpp");
    snippets_add(sm, "if", "Condition if", 
        "if condition:\n    \n", "python");
    
    snippets_add(sm, "else", "Else", 
        "else {\n    \n}", "c");
    snippets_add(sm, "else", "Else", 
        "else {\n    \n}", "cpp");
    snippets_add(sm, "else", "Else", 
        "else:\n    \n", "python");
    
    snippets_add(sm, "while", "Boucle while", 
        "while (condition) {\n    \n}", "c");
    snippets_add(sm, "while", "Boucle while", 
        "while condition:\n    \n", "python");
    
    snippets_add(sm, "do", "Boucle do-while", 
        "do {\n    \n} while (condition);", "c");
    snippets_add(sm, "do", "Boucle do-while", 
        "do {\n    \n} while (condition);", "cpp");
    
    snippets_add(sm, "switch", "Switch", 
        "switch (value) {\n    case 0:\n        \n        break;\n    default:\n        break;\n}", "c");
    
    snippets_add(sm, "class", "Classe", 
        "class ClassName {\npublic:\n    ClassName() {}\n    ~ClassName() {}\nprivate:\n    \n};", "cpp");
    
    snippets_add(sm, "struct", "Structure", 
        "typedef struct {\n    \n} StructName;", "c");
    
    snippets_add(sm, "main", "Fonction main", 
        "int main(int argc, char* argv[]) {\n    \n    return 0;\n}", "c");
    snippets_add(sm, "main", "Fonction main", 
        "int main(int argc, char* argv[]) {\n    \n    return 0;\n}", "cpp");
    
    // Python Snippets
    snippets_add(sm, "def", "Définition de fonction", 
        "def function_name():\n    \n    return", "python");
    snippets_add(sm, "class", "Classe Python", 
        "class ClassName:\n    def __init__(self):\n        \n    def method(self):\n        \n", "python");
    snippets_add(sm, "try", "Try/Except", 
        "try:\n    \nexcept Exception as e:\n    \n", "python");
    snippets_add(sm, "with", "Context manager", 
        "with open('file.txt', 'r') as f:\n    \n", "python");
    
    // JavaScript Snippets
    snippets_add(sm, "function", "Fonction JS", 
        "function functionName() {\n    \n}", "javascript");
    snippets_add(sm, "arrow", "Fonction fléchée", 
        "const name = () => {\n    \n}", "javascript");
    snippets_add(sm, "class", "Classe JS", 
        "class ClassName {\n    constructor() {\n        \n    }\n    \n}", "javascript");
    snippets_add(sm, "promise", "Promise", 
        "new Promise((resolve, reject) => {\n    \n});", "javascript");
    snippets_add(sm, "async", "Fonction async", 
        "async function name() {\n    try {\n        \n    } catch (error) {\n        \n    }\n}", "javascript");
}

void snippets_add(SnippetManager* sm, const char* trigger, const char* desc, 
                  const char* code, const char* language) {
    Snippet* snippet = malloc(sizeof(Snippet));
    snippet->trigger = strdup(trigger);
    snippet->description = strdup(desc);
    snippet->code = strdup(code);
    snippet->language = language ? strdup(language) : NULL;
    snippet->next = sm->snippets;
    sm->snippets = snippet;
    sm->count++;
}

Snippet* snippets_find(SnippetManager* sm, const char* trigger, const char* language) {
    Snippet* current = sm->snippets;
    Snippet* best_match = NULL;
    
    while (current) {
        if (strcmp(current->trigger, trigger) == 0) {
            if (!current->language || !language) {
                if (!best_match) best_match = current;
            } else if (strcmp(current->language, language) == 0) {
                return current; // Match exact
            }
        }
        current = current->next;
    }
    
    return best_match;
}

void snippets_insert(Editor* editor, Snippet* snippet) {
    if (!snippet || !snippet->code) return;
    
    char* code = snippet->code;
    int len = strlen(code);
    
    // Insérer le code ligne par ligne
    char* line_start = code;
    char* line_end;
    int line_num = editor->cursor_y;
    int col_start = editor->cursor_x;
    
    while ((line_end = strchr(line_start, '\n')) != NULL) {
        *line_end = '\0';
        
        // Insérer la ligne
        for (int i = 0; line_start[i]; i++) {
            buffer_insert(editor->buffer, line_num, editor->cursor_x + i, line_start[i]);
        }
        
        // Aller à la ligne suivante
        editor->cursor_x += strlen(line_start);
        line_num++;
        editor->cursor_y = line_num;
        editor->cursor_x = col_start; // Garder l'indentation
        
        line_start = line_end + 1;
    }
    
    // Dernière ligne
    if (*line_start) {
        for (int i = 0; line_start[i]; i++) {
            buffer_insert(editor->buffer, line_num, editor->cursor_x + i, line_start[i]);
        }
        editor->cursor_x += strlen(line_start);
    }
    
    editor->modified = 1;
}

void snippets_render(SnippetManager* sm, int x, int y, int width, int height) {
    if (!sm || sm->count == 0) return;
    
    attron(A_BOLD);
    mvprintw(y, x, "📋 Snippets disponibles:");
    attroff(A_BOLD);
    
    int line = y + 1;
    Snippet* current = sm->snippets;
    int count = 0;
    
    while (current && line < y + height - 2) {
        mvprintw(line, x + 2, "  %s: %s", current->trigger, current->description);
        if (current->language) {
            attron(COLOR_PAIR(2));
            mvprintw(line, x + width - 15, "[%s]", current->language);
            attroff(COLOR_PAIR(2));
        }
        line++;
        current = current->next;
        count++;
        if (count >= 20) break;
    }
}