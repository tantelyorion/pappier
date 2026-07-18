#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "editor.h"
#include "ui.h"
#include "config.h"
#include "shortcuts.h"
#include "search.h"
#include "autocomplete.h"

static Config* global_config = NULL;
static ShortcutManager* global_shortcuts = NULL;

void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        // Sauvegarder la configuration
        if (global_config) {
            config_save(global_config, "pappier.conf");
            config_free(global_config);
        }
        if (global_shortcuts) {
            shortcuts_free(global_shortcuts);
        }
        exit(0);
    }
}

void print_banner(void) {
    printf("\033[2J\033[H");
    printf("\033[38;2;47;128;255m");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                ║\n");
    printf("║   ██████╗  █████╗ ██████╗ ██████╗ ██╗███████╗██████╗          ║\n");
    printf("║   ██╔══██╗██╔══██╗██╔══██╗██╔══██╗██║██╔════╝██╔══██╗         ║\n");
    printf("║   ██████╔╝███████║██████╔╝██████╔╝██║█████╗  ██████╔╝         ║\n");
    printf("║   ██╔═══╝ ██╔══██║██╔═══╝ ██╔══██╗██║██╔══╝  ██╔══██╗         ║\n");
    printf("║   ██║     ██║  ██║██║     ██║  ██║██║███████╗██║  ██║         ║\n");
    printf("║   ╚═╝     ╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝╚═╝╚══════╝╚═╝  ╚═╝         ║\n");
    printf("║                                                                ║\n");
    printf("║               Éditeur de Code Simple - v2.0                    ║\n");
    printf("║                                                                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m");
    printf("\nAppuyez sur une touche pour continuer...");
    getchar();
}

int main(int argc, char* argv[]) {
    // Gestion des signaux
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Afficher la bannière
    // print_banner();
    
    // Initialiser la configuration
    global_config = config_init();
    
    // Charger la configuration
    config_load(global_config, "pappier.conf");
    
    // Initialiser les raccourcis
    global_shortcuts = shortcuts_init();
    shortcuts_load(global_shortcuts, global_config);
    
    // Ajouter les raccourcis par défaut si non chargés
    if (global_shortcuts->count == 0) {
        shortcuts_add(global_shortcuts, "ctrl+s", "save", action_save);
        shortcuts_add(global_shortcuts, "ctrl+o", "open", action_open);
        shortcuts_add(global_shortcuts, "ctrl+f", "find", action_find);
        shortcuts_add(global_shortcuts, "ctrl+h", "replace", action_replace);
        shortcuts_add(global_shortcuts, "ctrl+z", "undo", action_undo);
        shortcuts_add(global_shortcuts, "ctrl+y", "redo", action_redo);
        shortcuts_add(global_shortcuts, "ctrl+a", "select_all", action_select_all);
        shortcuts_add(global_shortcuts, "ctrl+x", "cut", action_cut);
        shortcuts_add(global_shortcuts, "ctrl+c", "copy", action_copy);
        shortcuts_add(global_shortcuts, "ctrl+v", "paste", action_paste);
        shortcuts_add(global_shortcuts, "ctrl+q", "quit", action_quit);
    }
    
    // Initialiser l'interface utilisateur
    UI* ui = ui_init();
    ui->config = global_config;
    ui->shortcuts = global_shortcuts;
    
    // Charger le fichier si spécifié
    if (argc > 1) {
        editor_load_file(ui->editor, argv[1]);
    }
    
    // Appliquer la configuration
    config_apply(global_config);
    
    // Lancer l'éditeur
    ui_run(ui);
    
    // Nettoyer
    config_save(global_config, "pappier.conf");
    ui_free(ui);
    config_free(global_config);
    shortcuts_free(global_shortcuts);
    
    return 0;
}