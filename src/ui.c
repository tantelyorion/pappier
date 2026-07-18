#include "ui.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

// ============================================================================
// INITIALISATION ET NETTOYAGE
// ============================================================================

UI* ui_init(void) {
    UI* ui = malloc(sizeof(UI));
    if (!ui) return NULL;
    
    // Initialisation de l'éditeur
    ui->editor = editor_init();
    if (!ui->editor) {
        free(ui);
        return NULL;
    }
    
    // État de l'application
    ui->running = 1;
    ui->mode = 0;
    ui->command_buffer = malloc(512);
    if (!ui->command_buffer) {
        editor_free(ui->editor);
        free(ui);
        return NULL;
    }
    ui->command_buffer[0] = '\0';
    ui->command_len = 0;
    
    // Modules
    ui->search_ctx = search_init();
    ui->autocomplete = autocomplete_init();
    ui->config = config_init();
    ui->shortcuts = shortcuts_init();
    
    // Fonctionnalités avancées
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        ui->project = project_init(cwd);
        ui->git_status = git_init(cwd);
    } else {
        ui->project = project_init(".");
        ui->git_status = git_init(".");
    }
    
    ui->split_root = split_create(ui->editor);
    ui->compiler = compiler_init();
    ui->compile_result = NULL;
    ui->snippets = snippets_init();
    ui->macros = macros_init();
    ui->sessions = session_init();
    ui->theme_transparent = theme_transparent_init();
    
    // États d'affichage
    ui->show_project = 1;
    ui->show_git = 0;
    ui->show_terminal = 0;
    ui->show_snippets = 0;
    ui->show_macros = 0;
    ui->show_sessions = 0;
    ui->split_mode = 0;
    
    // États de l'éditeur
    ui->insert_mode = 0;
    ui->visual_mode = 0;
    ui->macro_recording = 0;
    ui->macro_name[0] = '\0';
    ui->has_selection = 0;
    ui->selection_start_line = 0;
    ui->selection_start_col = 0;
    ui->selection_end_line = 0;
    ui->selection_end_col = 0;
    
    // Sauvegarde automatique
    ui->last_save = time(NULL);
    ui->last_auto_save = time(NULL);
    
    // Dimensions
    ui->screen_width = 80;
    ui->screen_height = 24;
    ui->sidebar_width = 30;
    ui->bottom_height = 15;
    
    // Charger les raccourcis
    shortcuts_load(ui->shortcuts, ui->config);
    
    // Ajouter les raccourcis par défaut si non chargés
    if (ui->shortcuts->count == 0) {
        shortcuts_add(ui->shortcuts, "ctrl+s", "save", action_save);
        shortcuts_add(ui->shortcuts, "ctrl+o", "open", action_open);
        shortcuts_add(ui->shortcuts, "ctrl+f", "find", action_find);
        shortcuts_add(ui->shortcuts, "ctrl+h", "replace", action_replace);
        shortcuts_add(ui->shortcuts, "ctrl+z", "undo", action_undo);
        shortcuts_add(ui->shortcuts, "ctrl+y", "redo", action_redo);
        shortcuts_add(ui->shortcuts, "ctrl+a", "select_all", action_select_all);
        shortcuts_add(ui->shortcuts, "ctrl+x", "cut", action_cut);
        shortcuts_add(ui->shortcuts, "ctrl+c", "copy", action_copy);
        shortcuts_add(ui->shortcuts, "ctrl+v", "paste", action_paste);
        shortcuts_add(ui->shortcuts, "ctrl+q", "quit", action_quit);
        shortcuts_add(ui->shortcuts, "ctrl+b", "compile", action_compile);
        shortcuts_add(ui->shortcuts, "ctrl+r", "run", action_run);
        shortcuts_add(ui->shortcuts, "ctrl+g", "git", action_git_status);
        shortcuts_add(ui->shortcuts, "ctrl+p", "project", action_project_toggle);
        shortcuts_add(ui->shortcuts, "ctrl+\\", "split_v", action_split_vertical);
        shortcuts_add(ui->shortcuts, "ctrl+-", "split_h", action_split_horizontal);
        shortcuts_add(ui->shortcuts, "ctrl+`", "terminal", action_terminal);
        shortcuts_add(ui->shortcuts, "ctrl+t", "snippet", action_insert_snippet);
        shortcuts_add(ui->shortcuts, "ctrl+m", "macro_record", action_macro_record);
        shortcuts_add(ui->shortcuts, "ctrl+shift+m", "macro_play", action_macro_play);
        shortcuts_add(ui->shortcuts, "ctrl+shift+s", "session_save", action_session_save);
        shortcuts_add(ui->shortcuts, "ctrl+shift+l", "session_load", action_session_load);
        shortcuts_add(ui->shortcuts, "ctrl+shift+r", "session_restore", action_session_restore);
        shortcuts_add(ui->shortcuts, "ctrl+alt+t", "theme_transparency", action_theme_toggle_transparency);
        shortcuts_add(ui->shortcuts, "ctrl+alt+g", "theme_glass", action_theme_glass);
    }
    
    // Initialiser ncurses
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(1);
    start_color();
    use_default_colors();
    
    // Initialiser les paires de couleurs
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    
    return ui;
}

void ui_free(UI* ui) {
    if (!ui) return;
    
    // Libérer l'éditeur
    if (ui->editor) editor_free(ui->editor);
    
    // Libérer les modules
    if (ui->search_ctx) search_free(ui->search_ctx);
    if (ui->autocomplete) autocomplete_free(ui->autocomplete);
    if (ui->config) config_free(ui->config);
    if (ui->shortcuts) shortcuts_free(ui->shortcuts);
    if (ui->project) project_free(ui->project);
    if (ui->split_root) split_free(ui->split_root);
    if (ui->git_status) git_free(ui->git_status);
    if (ui->compiler) compiler_free(ui->compiler);
    if (ui->compile_result) {
        if (ui->compile_result->errors) free(ui->compile_result->errors);
        if (ui->compile_result->output) free(ui->compile_result->output);
        free(ui->compile_result);
    }
    if (ui->snippets) snippets_free(ui->snippets);
    if (ui->macros) macros_free(ui->macros);
    if (ui->sessions) session_free(ui->sessions);
    if (ui->theme_transparent) theme_transparent_free(ui->theme_transparent);
    
    // Libérer le buffer de commande
    if (ui->command_buffer) free(ui->command_buffer);
    
    // Fermer ncurses
    endwin();
    
    free(ui);
}

// ============================================================================
// BOUCLE PRINCIPALE
// ============================================================================

void ui_run(UI* ui) {
    if (!ui || !ui->editor) return;
    
    // Obtenir les dimensions de l'écran
    getmaxyx(stdscr, ui->screen_height, ui->screen_width);
    ui->editor->screen_width = ui->screen_width - ui->sidebar_width;
    ui->editor->screen_height = ui->screen_height - 3;
    
    // Boucle principale
    while (ui->running) {
        // Sauvegarde automatique
        time_t now = time(NULL);
        if (ui->editor->modified && 
            difftime(now, ui->last_auto_save) >= ui->config->backup_interval) {
            if (ui->editor->filename) {
                char backup_path[512];
                snprintf(backup_path, sizeof(backup_path), "%s/%s.bak",
                         ui->config->backup_dir,
                         strrchr(ui->editor->filename, '/') ? 
                         strrchr(ui->editor->filename, '/') + 1 : ui->editor->filename);
                editor_save_file(ui->editor, backup_path);
                ui->last_auto_save = now;
            }
        }
        
        // Rendu
        ui_render_all(ui);
        
        // Récupérer la touche
        int key = getch();
        ui_handle_input(ui, key);
    }
}

// ============================================================================
// GESTION DES ENTRÉES
// ============================================================================

void ui_handle_input(UI* ui, int key) {
    if (!ui) return;
    
    // Mode recherche
    if (ui->mode == 3) {
        if (key == 27) { // ESC
            ui_enter_normal_mode(ui);
            search_clear_highlights(ui->editor, ui->search_ctx);
        } else if (key == 10) { // Enter
            ui_enter_normal_mode(ui);
        } else if (key == KEY_BACKSPACE || key == 127) {
            search_incremental(ui->editor, ui->search_ctx, key);
        } else if (isprint(key)) {
            search_incremental(ui->editor, ui->search_ctx, key);
        }
        return;
    }
    
    // Mode commande
    if (ui->mode == 2) {
        if (key == 27) { // ESC
            ui_enter_normal_mode(ui);
            ui->command_buffer[0] = '\0';
            ui->command_len = 0;
        } else if (key == 10) { // Enter
            ui_handle_mode_command(ui, ui->command_buffer);
            ui_enter_normal_mode(ui);
            ui->command_buffer[0] = '\0';
            ui->command_len = 0;
        } else if (key == KEY_BACKSPACE || key == 127) {
            if (ui->command_len > 0) {
                ui->command_len--;
                ui->command_buffer[ui->command_len] = '\0';
            }
        } else if (isprint(key)) {
            ui->command_buffer[ui->command_len] = key;
            ui->command_len++;
            ui->command_buffer[ui->command_len] = '\0';
        }
        return;
    }
    
    // Mode normal
    if (ui->mode == 0) {
        switch(key) {
            case 'i':
                ui_enter_insert_mode(ui);
                break;
                
            case 'I':
                ui_enter_insert_mode(ui);
                ui->editor->cursor_x = 0;
                break;
                
            case 'a':
                ui_enter_insert_mode(ui);
                ui->editor->cursor_x++;
                break;
                
            case 'A':
                ui_enter_insert_mode(ui);
                ui->editor->cursor_x = buffer_line_length(ui->editor->buffer, 
                                                          ui->editor->cursor_y);
                break;
                
            case 'o':
                editor_insert_newline(ui->editor);
                ui_enter_insert_mode(ui);
                break;
                
            case 'O':
                editor_move_cursor(ui->editor, 0, -1);
                editor_insert_newline(ui->editor);
                ui->editor->cursor_y--;
                ui_enter_insert_mode(ui);
                break;
                
            case ':':
                ui_enter_command_mode(ui);
                break;
                
            case '/':
                ui_enter_search_mode(ui);
                search_clear_highlights(ui->editor, ui->search_ctx);
                break;
                
            case KEY_UP:
                editor_move_cursor(ui->editor, 0, -1);
                break;
            case KEY_DOWN:
                editor_move_cursor(ui->editor, 0, 1);
                break;
            case KEY_LEFT:
                editor_move_cursor(ui->editor, -1, 0);
                break;
            case KEY_RIGHT:
                editor_move_cursor(ui->editor, 1, 0);
                break;
                
            case KEY_HOME:
                editor_move_cursor(ui->editor, 0, 0);
                ui->editor->cursor_x = 0;
                break;
            case KEY_END:
                ui->editor->cursor_x = buffer_line_length(ui->editor->buffer,
                                                          ui->editor->cursor_y);
                break;
                
            case KEY_NPAGE:
                for (int i = 0; i < ui->screen_height - 4; i++) {
                    editor_move_cursor(ui->editor, 0, 1);
                }
                break;
            case KEY_PPAGE:
                for (int i = 0; i < ui->screen_height - 4; i++) {
                    editor_move_cursor(ui->editor, 0, -1);
                }
                break;
                
            case KEY_DC:
                editor_delete_char(ui->editor);
                break;
                
            case KEY_BACKSPACE:
            case 127:
                editor_move_cursor(ui->editor, -1, 0);
                editor_delete_char(ui->editor);
                break;
                
            case 10:
                editor_insert_newline(ui->editor);
                break;
                
            case 'd':
                if (key == 'd') {
                    buffer_delete_line(ui->editor->buffer, ui->editor->cursor_y);
                    if (ui->editor->cursor_y >= buffer_line_count(ui->editor->buffer)) {
                        ui->editor->cursor_y = buffer_line_count(ui->editor->buffer) - 1;
                    }
                    ui->editor->modified = 1;
                }
                break;
                
            case 'u':
                ui_undo(ui);
                break;
                
            case 'r':
                ui_redo(ui);
                break;
                
            case 's':
                ui_save_file(ui);
                break;
                
            case 'q':
                ui_quit(ui);
                break;
                
            case 'h':
                ui_handle_mode_command(ui, "help");
                break;
                
            case 'p':
                ui->show_project = !ui->show_project;
                break;
                
            case 'g':
                ui->show_git = !ui->show_git;
                break;
                
            default:
                // Vérifier les raccourcis personnalisés
                if (ui->shortcuts) {
                    char key_str[32];
                    if (key >= 32 && key <= 126) {
                        snprintf(key_str, sizeof(key_str), "%c", key);
                    } else {
                        snprintf(key_str, sizeof(key_str), "key_%d", key);
                    }
                    shortcuts_execute(ui->shortcuts, ui->editor, key_str);
                }
                break;
        }
        return;
    }
    
    // Mode insertion
    if (ui->mode == 1) {
        if (key == 27) { // ESC
            ui_enter_normal_mode(ui);
        } else if (key == KEY_BACKSPACE || key == 127) {
            if (ui->editor->cursor_x > 0) {
                editor_move_cursor(ui->editor, -1, 0);
                editor_delete_char(ui->editor);
            } else if (ui->editor->cursor_y > 0) {
                int prev_len = buffer_line_length(ui->editor->buffer, 
                                                  ui->editor->cursor_y - 1);
                buffer_merge_lines(ui->editor->buffer, 
                                   ui->editor->cursor_y - 1, 
                                   ui->editor->cursor_y);
                ui->editor->cursor_y--;
                ui->editor->cursor_x = prev_len;
                ui->editor->modified = 1;
            }
        } else if (key == KEY_UP) {
            editor_move_cursor(ui->editor, 0, -1);
        } else if (key == KEY_DOWN) {
            editor_move_cursor(ui->editor, 0, 1);
        } else if (key == KEY_LEFT) {
            editor_move_cursor(ui->editor, -1, 0);
        } else if (key == KEY_RIGHT) {
            editor_move_cursor(ui->editor, 1, 0);
        } else if (key == KEY_HOME) {
            ui->editor->cursor_x = 0;
        } else if (key == KEY_END) {
            ui->editor->cursor_x = buffer_line_length(ui->editor->buffer,
                                                      ui->editor->cursor_y);
        } else if (key == KEY_DC) {
            editor_delete_char(ui->editor);
        } else if (key == 10) {
            editor_insert_newline(ui->editor);
        } else if (isprint(key)) {
            editor_insert_char(ui->editor, key);
            
            // Mettre à jour l'autocomplétion
            if (ui->editor->cursor_x > 1) {
                autocomplete_update(ui->editor, ui->autocomplete);
            }
        }
    }
}

// ============================================================================
// GESTION DES COMMANDES
// ============================================================================

void ui_handle_mode_command(UI* ui, const char* command) {
    if (!ui || !command) return;
    
    // Quitter
    if (strcmp(command, "q") == 0 || strcmp(command, "quit") == 0) {
        ui->running = 0;
        return;
    }
    
    // Sauvegarder
    if (strcmp(command, "w") == 0) {
        ui_save_file(ui);
        return;
    }
    
    // Sauvegarder sous
    if (strncmp(command, "w ", 2) == 0) {
        char* filename = strdup(command + 2);
        editor_save_file(ui->editor, filename);
        free(filename);
        return;
    }
    
    // Ouvrir un fichier
    if (strncmp(command, "e ", 2) == 0) {
        char* filename = strdup(command + 2);
        editor_load_file(ui->editor, filename);
        free(filename);
        return;
    }
    
    // Rechercher
    if (strncmp(command, "find ", 5) == 0) {
        char* query = command + 5;
        search_find(ui->editor, ui->search_ctx, query);
        return;
    }
    
    // Remplacer
    if (strncmp(command, "replace ", 8) == 0) {
        char* rest = command + 8;
        char* space = strchr(rest, ' ');
        if (space) {
            *space = '\0';
            char* find = rest;
            char* replace = space + 1;
            ui->search_ctx->replace = strdup(replace);
            search_find(ui->editor, ui->search_ctx, find);
            search_replace_all(ui->editor, ui->search_ctx);
            if (ui->search_ctx->replace) {
                free(ui->search_ctx->replace);
                ui->search_ctx->replace = NULL;
            }
        }
        return;
    }
    
    // Compiler
    if (strcmp(command, "compile") == 0) {
        ui_compile(ui);
        return;
    }
    
    // Exécuter
    if (strcmp(command, "run") == 0) {
        ui_run_program(ui);
        return;
    }
    
    // Snippets
    if (strcmp(command, "snippets") == 0) {
        ui->show_snippets = !ui->show_snippets;
        ui->show_macros = 0;
        ui->show_sessions = 0;
        ui->show_terminal = 0;
        return;
    }
    
    if (strncmp(command, "snippet ", 8) == 0) {
        char* trigger = command + 8;
        char* lang = NULL;
        if (ui->editor->filename) {
            char* ext = strrchr(ui->editor->filename, '.');
            if (ext) {
                if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0) lang = "c";
                else if (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cc") == 0) lang = "cpp";
                else if (strcmp(ext, ".py") == 0) lang = "python";
                else if (strcmp(ext, ".js") == 0) lang = "javascript";
            }
        }
        Snippet* snippet = snippets_find(ui->snippets, trigger, lang);
        if (snippet) {
            snippets_insert(ui->editor, snippet);
        }
        return;
    }
    
    // Macros
    if (strcmp(command, "macros") == 0) {
        ui->show_macros = !ui->show_macros;
        ui->show_snippets = 0;
        ui->show_sessions = 0;
        ui->show_terminal = 0;
        return;
    }
    
    if (strncmp(command, "macro record ", 13) == 0) {
        char* name = command + 13;
        macros_start_recording(ui->macros, name);
        strcpy(ui->macro_name, name);
        ui->macro_recording = 1;
        return;
    }
    
    if (strcmp(command, "macro stop") == 0) {
        macros_stop_recording(ui->macros);
        ui->macro_recording = 0;
        return;
    }
    
    if (strncmp(command, "macro play ", 11) == 0) {
        char* name = command + 11;
        macros_play(ui->macros, ui->editor, name);
        return;
    }
    
    if (strncmp(command, "macro repeat ", 13) == 0) {
        char* rest = command + 13;
        char* space = strchr(rest, ' ');
        if (space) {
            *space = '\0';
            int times = atoi(rest);
            char* name = space + 1;
            macros_play_repeat(ui->macros, ui->editor, name, times);
        }
        return;
    }
    
    // Sessions
    if (strcmp(command, "sessions") == 0) {
        ui->show_sessions = !ui->show_sessions;
        ui->show_snippets = 0;
        ui->show_macros = 0;
        ui->show_terminal = 0;
        return;
    }
    
    if (strncmp(command, "session save ", 13) == 0) {
        char* name = command + 13;
        session_save(ui->sessions, ui->editor, ui->project, name);
        return;
    }
    
    if (strncmp(command, "session load ", 13) == 0) {
        char* name = command + 13;
        session_load(ui->sessions, ui->editor, ui->project, name);
        return;
    }
    
    if (strcmp(command, "session restore") == 0) {
        session_restore_last(ui->sessions, ui->editor, ui->project);
        return;
    }
    
    if (strncmp(command, "session delete ", 15) == 0) {
        char* name = command + 15;
        session_delete(ui->sessions, name);
        return;
    }
    
    // Split
    if (strcmp(command, "vsp") == 0 || strcmp(command, "split_v") == 0) {
        if (ui->split_root) {
            split_vertical(ui->split_root);
            ui->split_mode = 1;
        }
        return;
    }
    
    if (strcmp(command, "hsp") == 0 || strcmp(command, "split_h") == 0) {
        if (ui->split_root) {
            split_horizontal(ui->split_root);
            ui->split_mode = 1;
        }
        return;
    }
    
    if (strcmp(command, "close") == 0) {
        if (ui->split_mode && ui->split_root) {
            SplitPane* active = split_get_active(ui->split_root);
            if (active) {
                split_close(ui->split_root, active);
                if (ui->split_root->left == NULL && ui->split_root->right == NULL) {
                    ui->split_mode = 0;
                }
            }
        }
        return;
    }
    
    if (strcmp(command, "focus_next") == 0) {
        if (ui->split_root) {
            split_focus_next(ui->split_root);
        }
        return;
    }
    
    if (strcmp(command, "focus_prev") == 0) {
        if (ui->split_root) {
            split_focus_prev(ui->split_root);
        }
        return;
    }
    
    // Thème
    if (strcmp(command, "theme glass") == 0) {
        theme_transparent_set_glass(ui->theme_transparent, 0.7);
        return;
    }
    
    if (strcmp(command, "theme opacity") == 0) {
        theme_transparent_set_opacity(ui->theme_transparent, 200);
        return;
    }
    
    if (strcmp(command, "theme blur") == 0) {
        theme_transparent_set_blur(ui->theme_transparent, 1);
        return;
    }
    
    if (strcmp(command, "theme noblur") == 0) {
        theme_transparent_set_blur(ui->theme_transparent, 0);
        return;
    }
    
    // Git
    if (strcmp(command, "git") == 0) {
        ui->show_git = !ui->show_git;
        return;
    }
    
    if (strncmp(command, "git add ", 8) == 0) {
        char* file = command + 8;
        git_add(ui->git_status, file);
        return;
    }
    
    if (strncmp(command, "git commit ", 11) == 0) {
        char* message = command + 11;
        git_commit(ui->git_status, message);
        return;
    }
    
    if (strncmp(command, "git checkout ", 13) == 0) {
        char* branch = command + 13;
        git_checkout(ui->git_status, branch);
        return;
    }
    
    if (strcmp(command, "git pull") == 0) {
        git_pull(ui->git_status);
        return;
    }
    
    if (strcmp(command, "git push") == 0) {
        git_push(ui->git_status);
        return;
    }
    
    // Terminal
    if (strcmp(command, "terminal") == 0) {
        ui->show_terminal = !ui->show_terminal;
        ui->show_snippets = 0;
        ui->show_macros = 0;
        ui->show_sessions = 0;
        return;
    }
    
    // Help
    if (strcmp(command, "help") == 0) {
        clear();
        attron(A_BOLD);
        mvprintw(0, 0, "╔════════════════════════════════════════════════════════════════╗");
        mvprintw(1, 0, "║                     Pappier - Aide                             ║");
        mvprintw(2, 0, "╠════════════════════════════════════════════════════════════════╣");
        attroff(A_BOLD);
        
        int line = 3;
        mvprintw(line++, 0, "║  :q            Quitter                                        ║");
        mvprintw(line++, 0, "║  :w            Sauvegarder                                    ║");
        mvprintw(line++, 0, "║  :w <fichier>  Sauvegarder sous                               ║");
        mvprintw(line++, 0, "║  :e <fichier>  Ouvrir un fichier                              ║");
        mvprintw(line++, 0, "║  :find <mot>   Rechercher                                     ║");
        mvprintw(line++, 0, "║  :replace <anc> <nouveau>  Remplacer tout                     ║");
        mvprintw(line++, 0, "║  :compile      Compiler le fichier                            ║");
        mvprintw(line++, 0, "║  :run          Exécuter le programme                          ║");
        mvprintw(line++, 0, "║  :vsp          Split vertical                                 ║");
        mvprintw(line++, 0, "║  :hsp          Split horizontal                               ║");
        mvprintw(line++, 0, "║  :close        Fermer le panneau                              ║");
        mvprintw(line++, 0, "║  :snippets     Afficher les snippets                          ║");
        mvprintw(line++, 0, "║  :snippet <nom>  Insérer un snippet                          ║");
        mvprintw(line++, 0, "║  :macros       Afficher les macros                            ║");
        mvprintw(line++, 0, "║  :macro record <nom>  Enregistrer une macro                   ║");
        mvprintw(line++, 0, "║  :macro stop   Arrêter l'enregistrement                       ║");
        mvprintw(line++, 0, "║  :macro play <nom>  Jouer une macro                           ║");
        mvprintw(line++, 0, "║  :sessions     Afficher les sessions                          ║");
        mvprintw(line++, 0, "║  :session save <nom>  Sauvegarder la session                  ║");
        mvprintw(line++, 0, "║  :session load <nom>  Charger une session                     ║");
        mvprintw(line++, 0, "║  :session restore  Restaurer la dernière session              ║");
        mvprintw(line++, 0, "║  :theme glass  Activer l'effet verre                          ║");
        mvprintw(line++, 0, "║  :theme opacity  Régler l'opacité                             ║");
        mvprintw(line++, 0, "║  :git          Afficher le statut Git                         ║");
        mvprintw(line++, 0, "║  :git add <fichier>  Ajouter un fichier                       ║");
        mvprintw(line++, 0, "║  :git commit <message>  Commit                                ║");
        mvprintw(line++, 0, "║  :git pull     Git pull                                       ║");
        mvprintw(line++, 0, "║  :git push     Git push                                       ║");
        mvprintw(line++, 0, "║  :terminal     Afficher le terminal                           ║");
        mvprintw(line, 0, "╚════════════════════════════════════════════════════════════════╝");
        
        refresh();
        getch();
        return;
    }
}

// ============================================================================
// RENDU
// ============================================================================

void ui_render_all(UI* ui) {
    if (!ui) return;
    
    clear();
    
    // Mettre à jour les dimensions
    getmaxyx(stdscr, ui->screen_height, ui->screen_width);
    ui->sidebar_width = ui->show_project ? 30 : 0;
    ui->bottom_height = (ui->show_snippets || ui->show_macros || 
                         ui->show_sessions || ui->show_terminal) ? 15 : 0;
    
    int editor_start_x = ui->sidebar_width;
    int editor_width = ui->screen_width - ui->sidebar_width;
    int editor_height = ui->screen_height - 3 - ui->bottom_height;
    
    if (editor_width < 10) editor_width = 10;
    if (editor_height < 5) editor_height = 5;
    
    // Sidebar: Projet
    if (ui->show_project && ui->project) {
        project_render(ui->project, 0, 1, ui->sidebar_width, editor_height);
    }
    
    // Zone principale: Éditeur
    if (ui->split_mode && ui->split_root) {
        split_render_all(ui->split_root, editor_start_x, 1, 
                       editor_width, editor_height);
    } else if (ui->editor) {
        ui->editor->screen_width = editor_width;
        ui->editor->screen_height = editor_height;
        editor_render(ui->editor);
    }
    
    // Panneau du bas
    if (ui->bottom_height > 0) {
        ui_draw_bottom_panel(ui);
    }
    
    // Git status (overlay)
    if (ui->show_git && ui->git_status) {
        git_render_status(ui->git_status, ui->screen_width - 45, 2);
    }
    
    // Autocomplétion
    if (ui->mode == 1 && ui->autocomplete->count > 0) {
        ui_draw_autocomplete(ui);
    }
    
    // Minimap
    if (ui->config && ui->config->show_minimap) {
        ui_draw_minimap(ui);
    }
    
    // Barre de statut
    ui_draw_status_bar(ui);
    
    // Indicateur de mode
    ui_draw_mode_indicator(ui);
    
    // Ligne de commande
    if (ui->mode == 2) {
        ui_draw_command_line(ui);
    }
    
    // Ligne de recherche
    if (ui->mode == 3) {
        ui_draw_search_line(ui);
    }
    
    refresh();
}

void ui_draw_status_bar(UI* ui) {
    if (!ui || !ui->config || !ui->config->show_status_bar) return;
    
    attron(A_REVERSE);
    
    char status[512];
    const char* mode_text[] = {"NORMAL", "INSERT", "COMMAND", "SEARCH"};
    const char* mode = mode_text[ui->mode];
    
    char filename[100];
    if (ui->editor->filename) {
        char* basename = strrchr(ui->editor->filename, '/');
        snprintf(filename, sizeof(filename), "%s", basename ? basename + 1 : ui->editor->filename);
    } else {
        strcpy(filename, "[Nouveau]");
    }
    
    int line = ui->editor->cursor_y + 1;
    int col = ui->editor->cursor_x + 1;
    int total_lines = buffer_line_count(ui->editor->buffer);
    int modified = ui->editor->modified ? "*" : " ";
    int matches = ui->search_ctx->match_count;
    
    snprintf(status, sizeof(status), " %s %s%s  L%d:%d  %d%%  %s  %s",
             mode,
             filename,
             modified,
             line, col,
             (total_lines > 0) ? (line * 100 / total_lines) : 0,
             matches > 0 ? "[MATCH]" : "",
             ui->macro_recording ? "[REC]" : "");
    
    mvprintw(ui->screen_height - 1, 0, "%-*s", ui->screen_width, status);
    attroff(A_REVERSE);
}

void ui_draw_sidebar(UI* ui) {
    // Déjà géré dans ui_render_all
}

void ui_draw_bottom_panel(UI* ui) {
    if (!ui) return;
    
    int y = ui->screen_height - 3 - ui->bottom_height;
    int x = ui->sidebar_width;
    int width = ui->screen_width - ui->sidebar_width;
    int height = ui->bottom_height;
    
    // Séparateur
    attron(COLOR_PAIR(7));
    mvhline(y, x, ACS_HLINE, width);
    attroff(COLOR_PAIR(7));
    
    if (ui->show_snippets && ui->snippets) {
        snippets_render(ui->snippets, x + 2, y + 1, width - 4, height - 2);
    } else if (ui->show_macros && ui->macros) {
        macros_render(ui->macros, x + 2, y + 1, width - 4, height - 2);
    } else if (ui->show_sessions && ui->sessions) {
        session_render(ui->sessions, x + 2, y + 1, width - 4, height - 2);
    } else if (ui->show_terminal && ui->compile_result) {
        compiler_render_output(ui->compile_result, x + 2, y + 1, width - 4, height - 2);
    }
}

void ui_draw_mode_indicator(UI* ui) {
    if (!ui) return;
    
    const char* mode_str[] = {"NORMAL", "INSERT", "COMMAND", "SEARCH"};
    char extra[32] = "";
    
    if (ui->macro_recording) strcat(extra, " [REC]");
    if (ui->has_selection) strcat(extra, " [VISUAL]");
    if (ui->split_mode) strcat(extra, " [SPLIT]");
    
    attron(COLOR_PAIR(2));
    mvprintw(0, ui->screen_width - 30, "[%s%s]", mode_str[ui->mode], extra);
    attroff(COLOR_PAIR(2));
}

void ui_draw_command_line(UI* ui) {
    if (!ui) return;
    
    attron(A_REVERSE);
    char cmd_display[512];
    snprintf(cmd_display, sizeof(cmd_display), ":%s", ui->command_buffer);
    mvprintw(ui->screen_height - 1, 0, "%-*s", ui->screen_width, cmd_display);
    attroff(A_REVERSE);
}

void ui_draw_search_line(UI* ui) {
    if (!ui) return;
    
    attron(A_REVERSE);
    char search_display[512];
    snprintf(search_display, sizeof(search_display), "/%s", 
             ui->search_ctx->query ? ui->search_ctx->query : "");
    mvprintw(ui->screen_height - 1, 0, "%-*s", ui->screen_width, search_display);
    attroff(A_REVERSE);
}

void ui_draw_minimap(UI* ui) {
    if (!ui || !ui->editor) return;
    
    int minimap_width = 10;
    int y_start = 1;
    int y_end = ui->screen_height - 4;
    int total_lines = buffer_line_count(ui->editor->buffer);
    
    if (total_lines == 0) return;
    
    int visible_height = y_end - y_start;
    int start_line = 0;
    
    if (total_lines > visible_height) {
        start_line = (ui->editor->scroll_y * visible_height) / total_lines;
    }
    
    int x = ui->screen_width - minimap_width - 1;
    
    for (int i = 0; i < visible_height && i + start_line < total_lines; i++) {
        int line_idx = i + start_line;
        char* line = buffer_get_line(ui->editor->buffer, line_idx);
        int len = line ? strlen(line) : 0;
        
        if (len > 0) {
            int bar_len = (len * minimap_width) / 80;
            if (bar_len > minimap_width) bar_len = minimap_width;
            if (bar_len < 1) bar_len = 1;
            
            // Vérifier si la ligne a des matches
            int has_match = 0;
            for (int m = 0; m < ui->search_ctx->match_count; m++) {
                if (ui->search_ctx->match_lines[m] == line_idx) {
                    has_match = 1;
                    break;
                }
            }
            
            if (has_match) {
                attron(COLOR_PAIR(3));
            } else if (line_idx == ui->editor->cursor_y) {
                attron(COLOR_PAIR(1));
            } else {
                attron(COLOR_PAIR(7));
            }
            
            for (int j = 0; j < bar_len && j < minimap_width; j++) {
                mvaddch(y_start + i, x + j, ' ');
            }
            attroff(COLOR_PAIR(7));
            attroff(COLOR_PAIR(1));
            attroff(COLOR_PAIR(3));
        }
    }
}

void ui_draw_autocomplete(UI* ui) {
    if (!ui || !ui->autocomplete) return;
    
    Autocomplete* ac = ui->autocomplete;
    if (ac->count == 0) return;
    
    int x = ui->sidebar_width + ui->editor->cursor_x + 6;
    int y = ui->editor->cursor_y - ui->editor->scroll_y + 2;
    
    if (y > ui->screen_height - ac->count - 4) {
        y = ui->screen_height - ac->count - 4;
    }
    if (y < 1) y = 1;
    
    int max_display = 10;
    if (ac->count > max_display) max_display = ac->count;
    
    for (int i = 0; i < ac->count && i < max_display; i++) {
        if (i == ac->selected) {
            attron(A_REVERSE);
        }
        mvprintw(y + i, x, "%-20s", ac->suggestions[i]);
        if (i == ac->selected) {
            attroff(A_REVERSE);
        }
    }
}

// ============================================================================
// GESTION DES MODES
// ============================================================================

void ui_enter_normal_mode(UI* ui) {
    if (!ui) return;
    ui->mode = 0;
    curs_set(1);
}

void ui_enter_insert_mode(UI* ui) {
    if (!ui) return;
    ui->mode = 1;
    curs_set(2);
}

void ui_enter_command_mode(UI* ui) {
    if (!ui) return;
    ui->mode = 2;
    ui->command_buffer[0] = '\0';
    ui->command_len = 0;
}

void ui_enter_search_mode(UI* ui) {
    if (!ui) return;
    ui->mode = 3;
}

// ============================================================================
// ACTIONS
// ============================================================================

void ui_save_file(UI* ui) {
    if (!ui || !ui->editor) return;
    
    if (ui->editor->filename) {
        editor_save_file(ui->editor, ui->editor->filename);
        ui->last_save = time(NULL);
    } else {
        // Demander un nom de fichier
        ui_enter_command_mode(ui);
        strcpy(ui->command_buffer, "w ");
        ui->command_len = 2;
    }
}

void ui_open_file(UI* ui) {
    if (!ui) return;
    ui_enter_command_mode(ui);
    strcpy(ui->command_buffer, "e ");
    ui->command_len = 2;
}

void ui_find_text(UI* ui) {
    if (!ui) return;
    ui_enter_search_mode(ui);
}

void ui_replace_text(UI* ui) {
    if (!ui) return;
    ui_enter_command_mode(ui);
    strcpy(ui->command_buffer, "replace ");
    ui->command_len = 8;
}

void ui_undo(UI* ui) {
    if (!ui || !ui->editor) return;
    action_undo(ui->editor);
}

void ui_redo(UI* ui) {
    if (!ui || !ui->editor) return;
    action_redo(ui->editor);
}

void ui_select_all(UI* ui) {
    if (!ui || !ui->editor) return;
    action_select_all(ui->editor);
}

void ui_cut(UI* ui) {
    if (!ui || !ui->editor) return;
    action_cut(ui->editor);
}

void ui_copy(UI* ui) {
    if (!ui || !ui->editor) return;
    action_copy(ui->editor);
}

void ui_paste(UI* ui) {
    if (!ui || !ui->editor) return;
    action_paste(ui->editor);
}

void ui_quit(UI* ui) {
    if (!ui) return;
    ui->running = 0;
}

void ui_compile(UI* ui) {
    if (!ui || !ui->editor || !ui->editor->filename) return;
    
    if (ui->compile_result) {
        if (ui->compile_result->errors) free(ui->compile_result->errors);
        if (ui->compile_result->output) free(ui->compile_result->output);
        free(ui->compile_result);
    }
    
    ui->compile_result = compiler_compile(ui->compiler, ui->editor->filename);
    ui->show_terminal = 1;
    ui->show_snippets = 0;
    ui->show_macros = 0;
    ui->show_sessions = 0;
}

void ui_run_program(UI* ui) {
    if (!ui || !ui->compile_result || !ui->compile_result->success) {
        ui_compile(ui);
        if (!ui->compile_result || !ui->compile_result->success) {
            return;
        }
    }
    
    if (ui->compile_result && ui->compile_result->output) {
        clear();
        mvprintw(0, 0, "Exécution de %s...", ui->compile_result->output);
        mvprintw(1, 0, "Appuyez sur une touche pour continuer");
        refresh();
        getch();
        
        // Exécuter dans un terminal séparé
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "xterm -e './%s' &", ui->compile_result->output);
        system(cmd);
    }
}

// ============================================================================
// FONCTIONS POUR LES RACCOURCIS EXTERNES
// ============================================================================

void action_compile(Editor* editor) {
    // Cette fonction est appelée par les raccourcis
    // Elle doit accéder à l'UI, donc on utilise un pointeur global
    // Dans la vraie implémentation, on passerait l'UI en paramètre
}

void action_run(Editor* editor) {
    // Similaire à action_compile
}

void action_git_status(Editor* editor) {
    // Similaire
}

void action_project_toggle(Editor* editor) {
    // Similaire
}

void action_split_vertical(Editor* editor) {
    // Similaire
}

void action_split_horizontal(Editor* editor) {
    // Similaire
}

void action_terminal(Editor* editor) {
    // Similaire
}

void action_insert_snippet(Editor* editor) {
    // Similaire
}

void action_macro_record(Editor* editor) {
    // Similaire
}

void action_macro_play(Editor* editor) {
    // Similaire
}

void action_macro_stop(Editor* editor) {
    // Similaire
}

void action_session_save(Editor* editor) {
    // Similaire
}

void action_session_load(Editor* editor) {
    // Similaire
}

void action_session_restore(Editor* editor) {
    // Similaire
}

void action_theme_toggle_transparency(Editor* editor) {
    // Similaire
}

void action_theme_glass(Editor* editor) {
    // Similaire
}