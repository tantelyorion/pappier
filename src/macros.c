#include "macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

MacroManager* macros_init(void) {
    MacroManager* mm = malloc(sizeof(MacroManager));
    mm->macros = NULL;
    mm->count = 0;
    mm->current_recording = NULL;
    mm->current_playing = NULL;
    mm->repeat_count = 1;
    return mm;
}

void macros_free(MacroManager* mm) {
    if (!mm) return;
    
    Macro* current = mm->macros;
    while (current) {
        Macro* next = current->next;
        if (current->name) free(current->name);
        MacroAction* action = current->actions;
        while (action) {
            MacroAction* next_action = action->next;
            if (action->description) free(action->description);
            free(action);
            action = next_action;
        }
        free(current);
        current = next;
    }
    free(mm);
}

void macros_start_recording(MacroManager* mm, const char* name) {
    // Arrêter l'enregistrement en cours
    if (mm->current_recording) {
        macros_stop_recording(mm);
    }
    
    // Créer une nouvelle macro
    Macro* macro = malloc(sizeof(Macro));
    macro->name = strdup(name);
    macro->actions = NULL;
    macro->action_count = 0;
    macro->is_recording = 1;
    macro->is_playing = 0;
    macro->next = mm->macros;
    mm->macros = macro;
    mm->count++;
    mm->current_recording = macro;
}

void macros_stop_recording(MacroManager* mm) {
    if (mm->current_recording) {
        mm->current_recording->is_recording = 0;
        mm->current_recording = NULL;
    }
}

void macros_record_key(MacroManager* mm, int key) {
    if (!mm->current_recording || !mm->current_recording->is_recording) return;
    
    MacroAction* action = malloc(sizeof(MacroAction));
    action->key = key;
    action->description = NULL;
    action->next = NULL;
    
    // Ajouter à la fin
    if (!mm->current_recording->actions) {
        mm->current_recording->actions = action;
    } else {
        MacroAction* last = mm->current_recording->actions;
        while (last->next) last = last->next;
        last->next = action;
    }
    mm->current_recording->action_count++;
}

void macros_play(MacroManager* mm, Editor* editor, const char* name) {
    // Trouver la macro
    Macro* macro = mm->macros;
    while (macro) {
        if (strcmp(macro->name, name) == 0) break;
        macro = macro->next;
    }
    
    if (!macro || macro->action_count == 0) return;
    
    mm->current_playing = macro;
    macro->is_playing = 1;
    
    // Rejouer les actions
    MacroAction* action = macro->actions;
    while (action) {
        // Exécuter l'action (simulé)
        // Dans la vraie implémentation, on appellerait ui_handle_input
        action = action->next;
    }
    
    macro->is_playing = 0;
    mm->current_playing = NULL;
}

void macros_play_repeat(MacroManager* mm, Editor* editor, const char* name, int times) {
    for (int i = 0; i < times; i++) {
        macros_play(mm, editor, name);
    }
}

void macros_delete(MacroManager* mm, const char* name) {
    Macro* current = mm->macros;
    Macro* prev = NULL;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                mm->macros = current->next;
            }
            if (current->name) free(current->name);
            MacroAction* action = current->actions;
            while (action) {
                MacroAction* next = action->next;
                if (action->description) free(action->description);
                free(action);
                action = next;
            }
            free(current);
            mm->count--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

void macros_list(MacroManager* mm) {
    Macro* current = mm->macros;
    int i = 1;
    
    printf("\n📋 Macros enregistrées:\n");
    while (current) {
        printf("  %d. %s (%d actions)\n", i++, current->name, current->action_count);
        current = current->next;
    }
}

void macros_save(MacroManager* mm, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;
    
    Macro* current = mm->macros;
    while (current) {
        fprintf(file, "macro %s\n", current->name);
        MacroAction* action = current->actions;
        while (action) {
            fprintf(file, "  key %d\n", action->key);
            action = action->next;
        }
        fprintf(file, "end\n");
        current = current->next;
    }
    
    fclose(file);
}

void macros_load(MacroManager* mm, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;
    
    char line[256];
    Macro* current = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        
        if (strncmp(line, "macro ", 6) == 0) {
            char* name = line + 6;
            macros_start_recording(mm, name);
            current = mm->current_recording;
        } else if (strcmp(line, "end") == 0) {
            if (current) {
                current->is_recording = 0;
                current = NULL;
            }
            macros_stop_recording(mm);
        } else if (strncmp(line, "  key ", 6) == 0) {
            int key = atoi(line + 6);
            if (mm->current_recording) {
                macros_record_key(mm, key);
            }
        }
    }
    
    fclose(file);
}

void macros_render(MacroManager* mm, int x, int y, int width, int height) {
    if (!mm || mm->count == 0) return;
    
    attron(A_BOLD);
    mvprintw(y, x, "🎬 Macros disponibles:");
    attroff(A_BOLD);
    
    int line = y + 1;
    Macro* current = mm->macros;
    int count = 0;
    
    while (current && line < y + height - 2) {
        char status = current->is_recording ? '🔴' : (current->is_playing ? '▶️' : ' ');
        mvprintw(line, x + 2, "  %c %s (%d actions)", status, current->name, current->action_count);
        line++;
        current = current->next;
        count++;
        if (count >= 15) break;
    }
}