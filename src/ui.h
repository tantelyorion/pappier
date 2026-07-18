#ifndef UI_H
#define UI_H

#include <ncurses.h>
#include <time.h>
#include "editor.h"
#include "config.h"
#include "shortcuts.h"
#include "search.h"
#include "autocomplete.h"
#include "project.h"
#include "split.h"
#include "git.h"
#include "compiler.h"
#include "snippets.h"
#include "macros.h"
#include "session.h"
#include "theme_transparent.h"

typedef struct UI {
    // Éditeur principal
    Editor* editor;
    
    // État de l'application
    int running;
    int mode;                    // 0 = normal, 1 = insert, 2 = command, 3 = search
    char* command_buffer;
    int command_len;
    
    // Modules
    SearchContext* search_ctx;
    Autocomplete* autocomplete;
    Config* config;
    ShortcutManager* shortcuts;
    
    // Fonctionnalités avancées
    ProjectBrowser* project;
    SplitPane* split_root;
    GitStatus* git_status;
    CompilerConfig* compiler;
    CompileResult* compile_result;
    SnippetManager* snippets;
    MacroManager* macros;
    SessionManager* sessions;
    ThemeTransparent* theme_transparent;
    
    // États d'affichage
    int show_project;
    int show_git;
    int show_terminal;
    int show_snippets;
    int show_macros;
    int show_sessions;
    int split_mode;
    
    // États de l'éditeur
    int insert_mode;
    int visual_mode;
    int macro_recording;
    char macro_name[64];
    
    // Sélection
    int selection_start_line;
    int selection_start_col;
    int selection_end_line;
    int selection_end_col;
    int has_selection;
    
    // Sauvegarde automatique
    time_t last_save;
    time_t last_auto_save;
    
    // Dimensions
    int screen_width;
    int screen_height;
    int sidebar_width;
    int bottom_height;
} UI;

// Initialisation et nettoyage
UI* ui_init(void);
void ui_free(UI* ui);

// Boucle principale
void ui_run(UI* ui);

// Gestion des entrées
void ui_handle_input(UI* ui, int key);
void ui_handle_mode_command(UI* ui, const char* command);

// Rendu
void ui_render_all(UI* ui);
void ui_draw_status_bar(UI* ui);
void ui_draw_sidebar(UI* ui);
void ui_draw_bottom_panel(UI* ui);
void ui_draw_mode_indicator(UI* ui);
void ui_draw_command_line(UI* ui);
void ui_draw_search_line(UI* ui);
void ui_draw_minimap(UI* ui);
void ui_draw_autocomplete(UI* ui);

// Gestion des modes
void ui_enter_normal_mode(UI* ui);
void ui_enter_insert_mode(UI* ui);
void ui_enter_command_mode(UI* ui);
void ui_enter_search_mode(UI* ui);

// Actions
void ui_save_file(UI* ui);
void ui_open_file(UI* ui);
void ui_find_text(UI* ui);
void ui_replace_text(UI* ui);
void ui_undo(UI* ui);
void ui_redo(UI* ui);
void ui_select_all(UI* ui);
void ui_cut(UI* ui);
void ui_copy(UI* ui);
void ui_paste(UI* ui);
void ui_quit(UI* ui);
void ui_compile(UI* ui);
void ui_run_program(UI* ui);

#endif