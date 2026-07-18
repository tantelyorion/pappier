#include "editor.h"
#include "syntax.h"
#include "file.h"
#include "clipboard.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

Editor* editor_init(void) {
    Editor* editor = malloc(sizeof(Editor));
    if (!editor) return NULL;
    
    editor->buffer = buffer_init();
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    editor->scroll_x = 0;
    editor->scroll_y = 0;
    editor->screen_width = 80;
    editor->screen_height = 24;
    editor->modified = 0;
    editor->filename = NULL;
    
    theme_init(&editor->theme);
    
    return editor;
}

void editor_free(Editor* editor) {
    if (editor) {
        buffer_free(editor->buffer);
        if (editor->filename) free(editor->filename);
        free(editor);
    }
}

void editor_insert_char(Editor* editor, char c) {
    if (c == '\n') {
        editor_insert_newline(editor);
        return;
    }
    
    buffer_insert(editor->buffer, editor->cursor_y, editor->cursor_x, c);
    editor->cursor_x++;
    editor->modified = 1;
}

void editor_delete_char(Editor* editor) {
    if (editor->cursor_x > 0) {
        buffer_delete(editor->buffer, editor->cursor_y, editor->cursor_x - 1);
        editor->cursor_x--;
        editor->modified = 1;
    } else if (editor->cursor_y > 0) {
        // Joindre les lignes
        int prev_len = buffer_line_length(editor->buffer, editor->cursor_y - 1);
        buffer_merge_lines(editor->buffer, editor->cursor_y - 1, editor->cursor_y);
        editor->cursor_y--;
        editor->cursor_x = prev_len;
        editor->modified = 1;
    }
}

void editor_insert_newline(Editor* editor) {
    buffer_insert_line(editor->buffer, editor->cursor_y + 1);
    
    // Copier l'indentation de la ligne actuelle
    char* line = buffer_get_line(editor->buffer, editor->cursor_y);
    int indent = 0;
    while (line && (line[indent] == ' ' || line[indent] == '\t')) indent++;
    
    buffer_insert_text(editor->buffer, editor->cursor_y + 1, 0, line, indent);
    
    editor->cursor_y++;
    editor->cursor_x = indent;
    editor->modified = 1;
}

void editor_move_cursor(Editor* editor, int dx, int dy) {
    int new_y = editor->cursor_y + dy;
    if (new_y < 0 || new_y >= buffer_line_count(editor->buffer)) return;
    
    editor->cursor_y = new_y;
    
    int max_x = buffer_line_length(editor->buffer, editor->cursor_y);
    int new_x = editor->cursor_x + dx;
    if (new_x < 0) new_x = 0;
    if (new_x > max_x) new_x = max_x;
    editor->cursor_x = new_x;
}

void editor_move_word(Editor* editor, int direction) {
    char* line = buffer_get_line(editor->buffer, editor->cursor_y);
    if (!line) return;
    
    int pos = editor->cursor_x;
    int len = strlen(line);
    
    if (direction > 0) {
        // Avancer d'un mot
        while (pos < len && isalnum(line[pos])) pos++;
        while (pos < len && !isalnum(line[pos])) pos++;
    } else {
        // Reculer d'un mot
        while (pos > 0 && !isalnum(line[pos - 1])) pos--;
        while (pos > 0 && isalnum(line[pos - 1])) pos--;
    }
    
    editor->cursor_x = pos;
}

int editor_load_file(Editor* editor, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    
    buffer_clear(editor->buffer);
    
    char line[4096];
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
        buffer_append_line(editor->buffer, line);
    }
    
    fclose(file);
    
    if (editor->filename) free(editor->filename);
    editor->filename = strdup(filename);
    editor->modified = 0;
    
    return 0;
}

int editor_save_file(Editor* editor, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    
    for (int i = 0; i < buffer_line_count(editor->buffer); i++) {
        char* line = buffer_get_line(editor->buffer, i);
        fprintf(file, "%s\n", line);
    }
    
    fclose(file);
    
    if (editor->filename) free(editor->filename);
    editor->filename = strdup(filename);
    editor->modified = 0;
    
    return 0;
}

void editor_status_bar(Editor* editor) {
    // Rendu de la barre de statut
    const char* modified = editor->modified ? "*" : " ";
    const char* filename = editor->filename ? editor->filename : "[Nouveau fichier]";
    
    // Utiliser le thème
    printf("%s", theme_get_color_code(editor->theme.primary));
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    
    // Informations
    printf("║ %s%s", filename, modified);
    
    // Ligne/Colonne aligné à droite
    int line_col_info = 20;
    int info_pos = 60 - line_col_info;
    for (int i = strlen(filename) + 2; i < info_pos; i++) printf(" ");
    printf("Ligne %d, Col %d ", editor->cursor_y + 1, editor->cursor_x + 1);
    printf("║\n");
    
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m"); // Reset colors
}

void editor_render(Editor* editor) {
    // Effacer l'écran
    printf("\033[2J\033[H");
    
    // Appliquer le thème
    printf("%s", theme_get_color_code(editor->theme.bg_primary));
    printf("%s", theme_get_color_code(editor->theme.text_primary));
    
    // Afficher le texte
    int line_count = buffer_line_count(editor->buffer);
    int start_line = editor->scroll_y;
    int end_line = start_line + editor->screen_height - 2;
    if (end_line > line_count) end_line = line_count;
    
    for (int i = start_line; i < end_line; i++) {
        char* line = buffer_get_line(editor->buffer, i);
        if (!line) break;
        
        // Numéro de ligne (avec la couleur secondaire)
        printf("%s%4d ", theme_get_color_code(editor->theme.text_secondary), i + 1);
        
        // Contenu de la ligne
        printf("%s", theme_get_color_code(editor->theme.text_primary));
        
        // Coloration syntaxique
        if (editor->filename) {
            char* ext = strrchr(editor->filename, '.');
            if (ext && strcmp(ext, ".c") == 0) {
                syntax_highlight_c(line, &editor->theme);
            } else if (ext && strcmp(ext, ".py") == 0) {
                syntax_highlight_python(line, &editor->theme);
            } else if (ext && strcmp(ext, ".js") == 0) {
                syntax_highlight_javascript(line, &editor->theme);
            } else {
                printf("%s", line);
            }
        } else {
            printf("%s", line);
        }
        
        printf("\n");
    }
    
    // Barre de statut
    editor_status_bar(editor);
    
    // Positionner le curseur
    printf("\033[%d;%dH", editor->cursor_y - editor->scroll_y + 1, 
           editor->cursor_x + 6); // 6 pour le numéro de ligne
    
    fflush(stdout);
}