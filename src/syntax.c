#include "syntax.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// Fonction utilitaire pour vérifier si un mot est un mot-clé
static int is_keyword(const char* word, const char** keywords, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(word, keywords[i]) == 0) return 1;
    }
    return 0;
}

void syntax_highlight_c(const char* line, const Theme* theme) {
    static const char* keywords[] = {
        "if", "else", "for", "while", "do", "switch", "case",
        "return", "break", "continue", "goto", "default",
        "int", "char", "float", "double", "void", "long", "short",
        "struct", "enum", "union", "typedef", "sizeof",
        "const", "static", "extern", "register", "volatile"
    };
    static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
    
    // Parcourir la ligne
    int i = 0;
    int in_string = 0;
    int in_comment = 0;
    int in_char = 0;
    
    while (line[i]) {
        // Commentaire
        if (!in_string && !in_char && line[i] == '/' && line[i+1] == '/') {
            printf("%s%s", theme_get_color_code(theme->comment), line + i);
            return;
        }
        
        // Chaine de caractères
        if (!in_comment && !in_char && line[i] == '"') {
            in_string = !in_string;
            printf("%s%c", theme_get_color_code(theme->string), line[i]);
            i++;
            while (line[i] && (in_string || line[i] != '"')) {
                if (line[i] == '\\' && line[i+1]) {
                    printf("%c%c", line[i], line[i+1]);
                    i += 2;
                } else {
                    printf("%c", line[i]);
                    i++;
                }
            }
            if (line[i] == '"') {
                printf("%c", line[i]);
                i++;
            }
            printf("%s", theme_get_color_code(theme->text_primary));
            continue;
        }
        
        // Caractère littéral
        if (!in_string && !in_comment && line[i] == '\'') {
            in_char = !in_char;
            printf("%s%c", theme_get_color_code(theme->string), line[i]);
            i++;
            while (line[i] && (in_char || line[i] != '\'')) {
                printf("%c", line[i]);
                i++;
            }
            if (line[i] == '\'') {
                printf("%c", line[i]);
                i++;
            }
            printf("%s", theme_get_color_code(theme->text_primary));
            continue;
        }
        
        // Mots-clés
        if (!in_string && !in_comment && !in_char && isalpha(line[i])) {
            int start = i;
            while (isalnum(line[i]) || line[i] == '_') i++;
            int len = i - start;
            
            char word[64];
            strncpy(word, line + start, len);
            word[len] = '\0';
            
            if (is_keyword(word, keywords, keyword_count)) {
                printf("%s%s", theme_get_color_code(theme->keyword), word);
            } else {
                // Vérifier si c'est une fonction
                if (line[i] == '(') {
                    printf("%s%s", theme_get_color_code(theme->function), word);
                } else {
                    printf("%s%s", theme_get_color_code(theme->text_primary), word);
                }
            }
            
            printf("%s", theme_get_color_code(theme->text_primary));
            continue;
        }
        
        // Nombres
        if (!in_string && !in_comment && !in_char && isdigit(line[i])) {
            printf("%s", theme_get_color_code(theme->number));
            while (isdigit(line[i]) || line[i] == '.') {
                printf("%c", line[i]);
                i++;
            }
            printf("%s", theme_get_color_code(theme->text_primary));
            continue;
        }
        
        // Opérateurs
        if (!in_string && !in_comment && !in_char) {
            char ops[] = "=+-*/%&|<>!";
            if (strchr(ops, line[i])) {
                printf("%s%c", theme_get_color_code(theme->operator), line[i]);
                i++;
                printf("%s", theme_get_color_code(theme->text_primary));
                continue;
            }
        }
        
        // Caractère normal
        printf("%c", line[i]);
        i++;
    }
}

void syntax_highlight_python(const char* line, const Theme* theme) {
    static const char* keywords[] = {
        "if", "elif", "else", "for", "while", "try", "except",
        "finally", "with", "as", "import", "from", "class",
        "def", "return", "yield", "lambda", "global", "nonlocal",
        "True", "False", "None", "and", "or", "not", "in", "is"
    };
    static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
    
    // Implémentation similaire à C mais avec mots-clés Python
    // Code simplifié pour l'exemple
    printf("%s", line);
}

void syntax_highlight_javascript(const char* line, const Theme* theme) {
    static const char* keywords[] = {
        "if", "else", "for", "while", "do", "switch", "case",
        "return", "break", "continue", "throw", "try", "catch",
        "finally", "var", "let", "const", "function", "class",
        "import", "export", "default", "new", "this", "super",
        "extends", "implements", "interface", "typeof", "instanceof"
    };
    static const int keyword_count = sizeof(keywords) / sizeof(keywords[0]);
    
    // Implémentation similaire à C mais avec mots-clés JS
    printf("%s", line);
}