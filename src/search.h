#ifndef SEARCH_H
#define SEARCH_H

#include "editor.h"
#include <stdbool.h>

typedef struct {
    char* query;
    char* replace;
    int current_match;
    int match_count;
    int* match_lines;
    int* match_cols;
    bool case_sensitive;
    bool regex;
} SearchContext;

SearchContext* search_init(void);
void search_free(SearchContext* ctx);
void search_find(Editor* editor, SearchContext* ctx, const char* query);
void search_find_next(Editor* editor, SearchContext* ctx);
void search_find_prev(Editor* editor, SearchContext* ctx);
void search_replace(Editor* editor, SearchContext* ctx);
void search_replace_all(Editor* editor, SearchContext* ctx);
void search_highlight_matches(Editor* editor, SearchContext* ctx);

// Recherche incrémentale
void search_incremental(Editor* editor, SearchContext* ctx, char c);
void search_clear_highlights(Editor* editor, SearchContext* ctx);

#endif