#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    // Éditeur
    int tab_size;
    bool auto_indent;
    bool show_line_numbers;
    bool highlight_current_line;
    bool wrap_lines;
    int scroll_off;          // Nombre de lignes de marge de défilement
    
    // Thème
    char* theme_name;
    bool dark_mode;
    
    // Recherche
    bool search_highlight;
    bool case_sensitive;
    bool regex_search;
    
    // Interface
    bool show_status_bar;
    bool show_minimap;
    int font_size;
    
    // Sauvegarde
    int backup_interval;     // En secondes
    bool create_backup;
    char* backup_dir;
    
    // Raccourcis personnalisés
    char* keybindings[50];
} Config;

Config* config_init(void);
void config_free(Config* config);
int config_load(Config* config, const char* filename);
int config_save(Config* config, const char* filename);
void config_load_defaults(Config* config);
void config_apply(Config* config);

#endif