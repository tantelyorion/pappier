#include "shortcuts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// STRUCTURES POUR UNDO/REDO
// ============================================================================

typedef struct {
    char*** states;
    int* line_counts;
    int capacity;
    int top;
    int max_states;
} UndoStack;

static UndoStack* undo_stack = NULL;
static UndoStack* redo_stack = NULL;

// ============================================================================
// INITIALISATION ET NETTOYAGE
// ============================================================================

ShortcutManager* shortcuts_init(void) {
    ShortcutManager* sm = malloc(sizeof(ShortcutManager));
    if (!sm) return NULL;
    
    sm->shortcuts = malloc(sizeof(Shortcut) * 100);
    if (!sm->shortcuts) {
        free(sm);
        return NULL;
    }
    
    sm->count = 0;
    sm->capacity = 100;
    return sm;
}

void shortcuts_free(ShortcutManager* sm) {
    if (!sm) return;
    
    for (int i = 0; i < sm->count; i++) {
        if (sm->shortcuts[i].key) free(sm->shortcuts[i].key);
        if (sm->shortcuts[i].action) free(sm->shortcuts[i].action);
    }
    free(sm->shortcuts);
    free(sm);
}

// ============================================================================
// GESTION DES RACCOURCIS
// ============================================================================

void shortcuts_add(ShortcutManager* sm, const char* key, const char* action, 
                   void (*callback)(Editor*)) {
    if (!sm || sm->count >= sm->capacity) return;
    
    sm->shortcuts[sm->count].key = strdup(key);
    sm->shortcuts[sm->count].action = strdup(action);
    sm->shortcuts[sm->count].callback = callback;
    sm->count++;
}

void shortcuts_load(ShortcutManager* sm, Config* config) {
    if (!sm || !config) return;
    
    for (int i = 0; i < 50 && config->keybindings[i]; i++) {
        char* binding = config->keybindings[i];
        char* colon = strchr(binding, ':');
        if (!colon) continue;
        *colon = '\0';
        char* key = binding;
        char* action = colon + 1;
        
        // Supprimer les espaces
        while (isspace(*key)) key++;
        char* end_key = key + strlen(key) - 1;
        while (end_key > key && isspace(*end_key)) { *end_key = '\0'; end_key--; }
        
        while (isspace(*action)) action++;
        char* end_action = action + strlen(action) - 1;
        while (end_action > action && isspace(*end_action)) { *end_action = '\0'; end_action--; }
        
        // Associer aux actions
        if (strcmp(action, "save") == 0) {
            shortcuts_add(sm, key, action, action_save);
        } else if (strcmp(action, "open") == 0) {
            shortcuts_add(sm, key, action, action_open);
        } else if (strcmp(action, "find") == 0) {
            shortcuts_add(sm, key, action, action_find);
        } else if (strcmp(action, "replace") == 0) {
            shortcuts_add(sm, key, action, action_replace);
        } else if (strcmp(action, "undo") == 0) {
            shortcuts_add(sm, key, action, action_undo);
        } else if (strcmp(action, "redo") == 0) {
            shortcuts_add(sm, key, action, action_redo);
        } else if (strcmp(action, "select_all") == 0) {
            shortcuts_add(sm, key, action, action_select_all);
        } else if (strcmp(action, "cut") == 0) {
            shortcuts_add(sm, key, action, action_cut);
        } else if (strcmp(action, "copy") == 0) {
            shortcuts_add(sm, key, action, action_copy);
        } else if (strcmp(action, "paste") == 0) {
            shortcuts_add(sm, key, action, action_paste);
        } else if (strcmp(action, "quit") == 0) {
            shortcuts_add(sm, key, action, action_quit);
        } else if (strcmp(action, "compile") == 0) {
            shortcuts_add(sm, key, action, action_compile);
        } else if (strcmp(action, "run") == 0) {
            shortcuts_add(sm, key, action, action_run);
        } else if (strcmp(action, "git") == 0) {
            shortcuts_add(sm, key, action, action_git_status);
        } else if (strcmp(action, "project") == 0) {
            shortcuts_add(sm, key, action, action_project_toggle);
        } else if (strcmp(action, "split_v") == 0) {
            shortcuts_add(sm, key, action, action_split_vertical);
        } else if (strcmp(action, "split_h") == 0) {
            shortcuts_add(sm, key, action, action_split_horizontal);
        } else if (strcmp(action, "terminal") == 0) {
            shortcuts_add(sm, key, action, action_terminal);
        } else if (strcmp(action, "snippet") == 0) {
            shortcuts_add(sm, key, action, action_insert_snippet);
        } else if (strcmp(action, "macro_record") == 0) {
            shortcuts_add(sm, key, action, action_macro_record);
        } else if (strcmp(action, "macro_play") == 0) {
            shortcuts_add(sm, key, action, action_macro_play);
        } else if (strcmp(action, "macro_stop") == 0) {
            shortcuts_add(sm, key, action, action_macro_stop);
        } else if (strcmp(action, "session_save") == 0) {
            shortcuts_add(sm, key, action, action_session_save);
        } else if (strcmp(action, "session_load") == 0) {
            shortcuts_add(sm, key, action, action_session_load);
        } else if (strcmp(action, "session_restore") == 0) {
            shortcuts_add(sm, key, action, action_session_restore);
        } else if (strcmp(action, "theme_transparency") == 0) {
            shortcuts_add(sm, key, action, action_theme_toggle_transparency);
        } else if (strcmp(action, "theme_glass") == 0) {
            shortcuts_add(sm, key, action, action_theme_glass);
        }
    }
}

void shortcuts_execute(ShortcutManager* sm, Editor* editor, const char* key) {
    if (!sm || !editor || !key) return;
    
    for (int i = 0; i < sm->count; i++) {
        if (strcmp(sm->shortcuts[i].key, key) == 0) {
            if (sm->shortcuts[i].callback) {
                sm->shortcuts[i].callback(editor);
            }
            return;
        }
    }
}

// ============================================================================
// UNDO/REDO - GESTION DE L'HISTORIQUE
// ============================================================================

static void undo_init(void) {
    if (!undo_stack) {
        undo_stack = malloc(sizeof(UndoStack));
        undo_stack->capacity = 100;
        undo_stack->top = -1;
        undo_stack->max_states = 100;
        undo_stack->states = malloc(sizeof(char**) * undo_stack->capacity);
        undo_stack->line_counts = malloc(sizeof(int) * undo_stack->capacity);
    }
    if (!redo_stack) {
        redo_stack = malloc(sizeof(UndoStack));
        redo_stack->capacity = 100;
        redo_stack->top = -1;
        redo_stack->max_states = 100;
        redo_stack->states = malloc(sizeof(char**) * redo_stack->capacity);
        redo_stack->line_counts = malloc(sizeof(int) * redo_stack->capacity);
    }
}

static void save_state(Editor* editor, UndoStack* stack) {
    if (!editor || !stack) return;
    
    undo_init();
    
    if (stack->top < stack->max_states - 1) {
        stack->top++;
    } else {
        // Libérer l'état le plus ancien
        for (int i = 0; i < stack->line_counts[0]; i++) {
            free(stack->states[0][i]);
        }
        free(stack->states[0]);
        for (int i = 0; i < stack->top; i++) {
            stack->states[i] = stack->states[i + 1];
            stack->line_counts[i] = stack->line_counts[i + 1];
        }
        stack->top = stack->max_states - 1;
    }
    
    int line_count = buffer_line_count(editor->buffer);
    stack->line_counts[stack->top] = line_count;
    stack->states[stack->top] = malloc(sizeof(char*) * line_count);
    
    for (int i = 0; i < line_count; i++) {
        char* line = buffer_get_line(editor->buffer, i);
        if (line) {
            stack->states[stack->top][i] = strdup(line);
        } else {
            stack->states[stack->top][i] = strdup("");
        }
    }
}

static void restore_state(Editor* editor, UndoStack* stack) {
    if (!editor || !stack || stack->top < 0) return;
    
    // Sauvegarder l'état actuel pour redo
    save_state(editor, redo_stack);
    
    // Restaurer
    buffer_clear(editor->buffer);
    for (int i = 0; i < stack->line_counts[stack->top]; i++) {
        buffer_append_line(editor->buffer, stack->states[stack->top][i]);
    }
    
    editor->modified = 1;
    stack->top--;
}

void action_undo(Editor* editor) {
    if (!editor) return;
    
    undo_init();
    if (undo_stack->top >= 0) {
        restore_state(editor, undo_stack);
    }
}

void action_redo(Editor* editor) {
    if (!editor) return;
    
    undo_init();
    if (redo_stack->top >= 0) {
        restore_state(editor, redo_stack);
    }
}

// ============================================================================
// ACTIONS PRÉDÉFINIES
// ============================================================================

void action_save(Editor* editor) {
    if (!editor) return;
    
    if (editor->filename) {
        editor_save_file(editor, editor->filename);
    }
}

void action_open(Editor* editor) {
    if (!editor) return;
    
    // Dans la vraie implémentation, on affiche une boîte de dialogue
    printf("\033[2J\033[H");
    printf("Entrez le chemin du fichier: ");
    fflush(stdout);
    char filename[512];
    if (fgets(filename, sizeof(filename), stdin)) {
        filename[strcspn(filename, "\n")] = '\0';
        editor_load_file(editor, filename);
    }
}

void action_find(Editor* editor) {
    if (!editor) return;
    
    // Cette action est gérée par l'UI
    // Elle est appelée par les raccourcis mais l'UI gère l'affichage
}

void action_replace(Editor* editor) {
    if (!editor) return;
    // Gérée par l'UI
}

void action_select_all(Editor* editor) {
    if (!editor) return;
    // Sélectionner tout le texte
    editor->cursor_x = 0;
    editor->cursor_y = 0;
    // Dans la vraie implémentation, on stockerait la sélection
}

void action_cut(Editor* editor) {
    if (!editor) return;
    // Implémentation du couper
    // Copier le texte sélectionné puis le supprimer
}

void action_copy(Editor* editor) {
    if (!editor) return;
    // Implémentation du copier
    // Copier le texte sélectionné dans le presse-papiers
}

void action_paste(Editor* editor) {
    if (!editor) return;
    // Implémentation du coller
    // Insérer le texte du presse-papiers
}

void action_quit(Editor* editor) {
    if (!editor) return;
    // L'UI gère la fermeture
    exit(0);
}

// ============================================================================
// ACTIONS POUR LES FONCTIONNALITÉS AVANCÉES
// ============================================================================

void action_compile(Editor* editor) {
    if (!editor || !editor->filename) return;
    // Cette fonction est appelée par les raccourcis
    // L'UI gère la compilation via ui_compile()
}

void action_run(Editor* editor) {
    if (!editor) return;
    // L'UI gère l'exécution
}

void action_git_status(Editor* editor) {
    if (!editor) return;
    // L'UI gère l'affichage Git
}

void action_project_toggle(Editor* editor) {
    if (!editor) return;
    // L'UI gère l'affichage du projet
}

void action_split_vertical(Editor* editor) {
    if (!editor) return;
    // L'UI gère le split vertical
}

void action_split_horizontal(Editor* editor) {
    if (!editor) return;
    // L'UI gère le split horizontal
}

void action_terminal(Editor* editor) {
    if (!editor) return;
    // L'UI gère l'affichage du terminal
}

void action_insert_snippet(Editor* editor) {
    if (!editor) return;
    // L'UI gère l'insertion de snippets
}

void action_macro_record(Editor* editor) {
    if (!editor) return;
    // L'UI gère l'enregistrement de macros
}

void action_macro_play(Editor* editor) {
    if (!editor) return;
    // L'UI gère la lecture de macros
}

void action_macro_stop(Editor* editor) {
    if (!editor) return;
    // L'UI gère l'arrêt de l'enregistrement
}

void action_session_save(Editor* editor) {
    if (!editor) return;
    // L'UI gère la sauvegarde de session
}

void action_session_load(Editor* editor) {
    if (!editor) return;
    // L'UI gère le chargement de session
}

void action_session_restore(Editor* editor) {
    if (!editor) return;
    // L'UI gère la restauration de session
}

void action_theme_toggle_transparency(Editor* editor) {
    if (!editor) return;
    // L'UI gère la transparence
}

void action_theme_glass(Editor* editor) {
    if (!editor) return;
    // L'UI gère l'effet verre
}

// ============================================================================
// FONCTIONS UTILITAIRES
// ============================================================================

char* shortcut_key_to_string(int key) {
    static char buffer[64];
    
    if (key >= 32 && key <= 126) {
        snprintf(buffer, sizeof(buffer), "%c", key);
    } else if (key == KEY_UP) {
        strcpy(buffer, "up");
    } else if (key == KEY_DOWN) {
        strcpy(buffer, "down");
    } else if (key == KEY_LEFT) {
        strcpy(buffer, "left");
    } else if (key == KEY_RIGHT) {
        strcpy(buffer, "right");
    } else if (key == KEY_HOME) {
        strcpy(buffer, "home");
    } else if (key == KEY_END) {
        strcpy(buffer, "end");
    } else if (key == KEY_PPAGE) {
        strcpy(buffer, "pageup");
    } else if (key == KEY_NPAGE) {
        strcpy(buffer, "pagedown");
    } else if (key == KEY_DC) {
        strcpy(buffer, "delete");
    } else if (key == KEY_BACKSPACE || key == 127) {
        strcpy(buffer, "backspace");
    } else if (key == 10) {
        strcpy(buffer, "enter");
    } else if (key == 27) {
        strcpy(buffer, "esc");
    } else if (key == 9) {
        strcpy(buffer, "tab");
    } else {
        snprintf(buffer, sizeof(buffer), "key_%d", key);
    }
    
    return buffer;
}

void shortcuts_list(ShortcutManager* sm) {
    if (!sm) return;
    
    printf("\n📋 Raccourcis disponibles:\n");
    for (int i = 0; i < sm->count; i++) {
        printf("  %s -> %s\n", sm->shortcuts[i].key, sm->shortcuts[i].action);
    }
}

int shortcuts_save_to_file(ShortcutManager* sm, const char* filename) {
    if (!sm || !filename) return -1;
    
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    
    fprintf(file, "# Pappier Shortcuts File\n");
    fprintf(file, "# Format: key:action\n\n");
    
    for (int i = 0; i < sm->count; i++) {
        fprintf(file, "%s:%s\n", sm->shortcuts[i].key, sm->shortcuts[i].action);
    }
    
    fclose(file);
    return 0;
}

int shortcuts_load_from_file(ShortcutManager* sm, const char* filename) {
    if (!sm || !filename) return -1;
    
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '#' || line[0] == '\0') continue;
        
        char* colon = strchr(line, ':');
        if (!colon) continue;
        *colon = '\0';
        char* key = line;
        char* action = colon + 1;
        
        // Supprimer les espaces
        while (isspace(*key)) key++;
        char* end_key = key + strlen(key) - 1;
        while (end_key > key && isspace(*end_key)) { *end_key = '\0'; end_key--; }
        
        while (isspace(*action)) action++;
        char* end_action = action + strlen(action) - 1;
        while (end_action > action && isspace(*end_action)) { *end_action = '\0'; end_action--; }
        
        shortcuts_add(sm, key, action, NULL);
    }
    
    fclose(file);
    return 0;
}

// ============================================================================
// FIN DU FICHIER
// ============================================================================