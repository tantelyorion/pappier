#include "search.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

SearchContext* search_init(void) {
    SearchContext* ctx = malloc(sizeof(SearchContext));
    ctx->query = NULL;
    ctx->replace = NULL;
    ctx->current_match = -1;
    ctx->match_count = 0;
    ctx->match_lines = NULL;
    ctx->match_cols = NULL;
    ctx->case_sensitive = false;
    ctx->regex = false;
    return ctx;
}

void search_free(SearchContext* ctx) {
    if (ctx) {
        if (ctx->query) free(ctx->query);
        if (ctx->replace) free(ctx->replace);
        if (ctx->match_lines) free(ctx->match_lines);
        if (ctx->match_cols) free(ctx->match_cols);
        free(ctx);
    }
}

// Recherche simple (sans regex)
static int find_occurrence(const char* text, const char* query, int start, bool case_sensitive) {
    if (!text || !query || start < 0) return -1;
    
    int text_len = strlen(text);
    int query_len = strlen(query);
    
    for (int i = start; i <= text_len - query_len; i++) {
        bool match = true;
        for (int j = 0; j < query_len; j++) {
            char tc = case_sensitive ? text[i+j] : tolower(text[i+j]);
            char qc = case_sensitive ? query[j] : tolower(query[j]);
            if (tc != qc) { match = false; break; }
        }
        if (match) return i;
    }
    return -1;
}

void search_find(Editor* editor, SearchContext* ctx, const char* query) {
    if (ctx->query) free(ctx->query);
    ctx->query = strdup(query);
    ctx->match_count = 0;
    ctx->current_match = -1;
    
    // Libérer les anciens matches
    if (ctx->match_lines) free(ctx->match_lines);
    if (ctx->match_cols) free(ctx->match_cols);
    ctx->match_lines = NULL;
    ctx->match_cols = NULL;
    
    // Compter les occurrences
    int max_matches = 1000;
    ctx->match_lines = malloc(sizeof(int) * max_matches);
    ctx->match_cols = malloc(sizeof(int) * max_matches);
    
    for (int line = 0; line < buffer_line_count(editor->buffer); line++) {
        char* text = buffer_get_line(editor->buffer, line);
        int pos = 0;
        while ((pos = find_occurrence(text, query, pos, ctx->case_sensitive)) != -1) {
            ctx->match_lines[ctx->match_count] = line;
            ctx->match_cols[ctx->match_count] = pos;
            ctx->match_count++;
            pos += strlen(query);
        }
    }
    
    if (ctx->match_count > 0) {
        ctx->current_match = 0;
        // Déplacer le curseur vers le premier match
        editor->cursor_y = ctx->match_lines[0];
        editor->cursor_x = ctx->match_cols[0];
    }
}

void search_find_next(Editor* editor, SearchContext* ctx) {
    if (ctx->match_count == 0) return;
    ctx->current_match = (ctx->current_match + 1) % ctx->match_count;
    editor->cursor_y = ctx->match_lines[ctx->current_match];
    editor->cursor_x = ctx->match_cols[ctx->current_match];
}

void search_find_prev(Editor* editor, SearchContext* ctx) {
    if (ctx->match_count == 0) return;
    ctx->current_match = (ctx->current_match - 1 + ctx->match_count) % ctx->match_count;
    editor->cursor_y = ctx->match_lines[ctx->current_match];
    editor->cursor_x = ctx->match_cols[ctx->current_match];
}

void search_replace(Editor* editor, SearchContext* ctx) {
    if (ctx->match_count == 0 || ctx->current_match < 0) return;
    
    int line = ctx->match_lines[ctx->current_match];
    int col = ctx->match_cols[ctx->current_match];
    int query_len = strlen(ctx->query);
    int replace_len = strlen(ctx->replace);
    
    // Supprimer l'ancien texte
    for (int i = 0; i < query_len; i++) {
        buffer_delete(editor->buffer, line, col);
    }
    
    // Insérer le nouveau texte
    for (int i = 0; i < replace_len; i++) {
        buffer_insert(editor->buffer, line, col + i, ctx->replace[i]);
    }
    
    editor->modified = 1;
    editor->cursor_x = col + replace_len;
    
    // Recalculer les matches
    search_find(editor, ctx, ctx->query);
}

void search_replace_all(Editor* editor, SearchContext* ctx) {
    if (ctx->match_count == 0) return;
    
    int offset = 0;
    int query_len = strlen(ctx->query);
    int replace_len = strlen(ctx->replace);
    
    for (int i = 0; i < ctx->match_count; i++) {
        int line = ctx->match_lines[i];
        int col = ctx->match_cols[i] + offset;
        
        for (int j = 0; j < query_len; j++) {
            buffer_delete(editor->buffer, line, col);
        }
        
        for (int j = 0; j < replace_len; j++) {
            buffer_insert(editor->buffer, line, col + j, ctx->replace[j]);
        }
        
        offset += replace_len - query_len;
    }
    
    editor->modified = 1;
    search_find(editor, ctx, ctx->query);
}

void search_highlight_matches(Editor* editor, SearchContext* ctx) {
    // Implémentation pour afficher les surbrillances
    // (sera appelée par le renderer)
}

void search_incremental(Editor* editor, SearchContext* ctx, char c) {
    // Recherche en temps réel
    if (c == 127 || c == KEY_BACKSPACE) {
        if (ctx->query && strlen(ctx->query) > 0) {
            ctx->query[strlen(ctx->query) - 1] = '\0';
        }
    } else if (isprint(c)) {
        if (!ctx->query) {
            ctx->query = malloc(2);
            ctx->query[0] = c;
            ctx->query[1] = '\0';
        } else {
            char* new = malloc(strlen(ctx->query) + 2);
            strcpy(new, ctx->query);
            new[strlen(ctx->query)] = c;
            new[strlen(ctx->query) + 1] = '\0';
            free(ctx->query);
            ctx->query = new;
        }
    }
    
    if (ctx->query && strlen(ctx->query) > 0) {
        search_find(editor, ctx, ctx->query);
    }
}

void search_clear_highlights(Editor* editor, SearchContext* ctx) {
    ctx->match_count = 0;
    if (ctx->match_lines) {
        free(ctx->match_lines);
        ctx->match_lines = NULL;
    }
    if (ctx->match_cols) {
        free(ctx->match_cols);
        ctx->match_cols = NULL;
    }
}