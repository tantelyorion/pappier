#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void config_load_defaults(Config* config) {
    config->tab_size = 4;
    config->auto_indent = true;
    config->show_line_numbers = true;
    config->highlight_current_line = true;
    config->wrap_lines = false;
    config->scroll_off = 3;
    
    config->theme_name = strdup("vezo-dark");
    config->dark_mode = true;
    
    config->search_highlight = true;
    config->case_sensitive = false;
    config->regex_search = false;
    
    config->show_status_bar = true;
    config->show_minimap = false;
    config->font_size = 14;
    
    config->backup_interval = 30;
    config->create_backup = true;
    config->backup_dir = strdup(".pappier_backup");
    
    // Raccourcis par défaut
    config->keybindings[0] = strdup("ctrl+s:save");
    config->keybindings[1] = strdup("ctrl+o:open");
    config->keybindings[2] = strdup("ctrl+f:find");
    config->keybindings[3] = strdup("ctrl+h:replace");
    config->keybindings[4] = strdup("ctrl+z:undo");
    config->keybindings[5] = strdup("ctrl+y:redo");
    config->keybindings[6] = strdup("ctrl+a:select_all");
}

Config* config_init(void) {
    Config* config = malloc(sizeof(Config));
    config_load_defaults(config);
    return config;
}

void config_free(Config* config) {
    if (config) {
        if (config->theme_name) free(config->theme_name);
        if (config->backup_dir) free(config->backup_dir);
        for (int i = 0; i < 50; i++) {
            if (config->keybindings[i]) free(config->keybindings[i]);
        }
        free(config);
    }
}

int config_load(Config* config, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Ignorer les commentaires et lignes vides
        char* trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        if (*trimmed == '#' || *trimmed == '\0' || *trimmed == '\n') continue;
        
        // Parser key = value
        char* equals = strchr(line, '=');
        if (!equals) continue;
        *equals = '\0';
        
        char* key = line;
        char* value = equals + 1;
        
        // Supprimer les espaces
        while (isspace(*key)) key++;
        char* end_key = key + strlen(key) - 1;
        while (end_key > key && isspace(*end_key)) { *end_key = '\0'; end_key--; }
        
        while (isspace(*value)) value++;
        char* end_value = value + strlen(value) - 1;
        while (end_value > value && (isspace(*end_value) || *end_value == '\n')) {
            *end_value = '\0';
            end_value--;
        }
        
        // Appliquer les paramètres
        if (strcmp(key, "tab_size") == 0) config->tab_size = atoi(value);
        else if (strcmp(key, "auto_indent") == 0) config->auto_indent = strcmp(value, "true") == 0;
        else if (strcmp(key, "show_line_numbers") == 0) config->show_line_numbers = strcmp(value, "true") == 0;
        else if (strcmp(key, "highlight_current_line") == 0) config->highlight_current_line = strcmp(value, "true") == 0;
        else if (strcmp(key, "wrap_lines") == 0) config->wrap_lines = strcmp(value, "true") == 0;
        else if (strcmp(key, "scroll_off") == 0) config->scroll_off = atoi(value);
        else if (strcmp(key, "theme") == 0) {
            if (config->theme_name) free(config->theme_name);
            config->theme_name = strdup(value);
        }
        else if (strcmp(key, "dark_mode") == 0) config->dark_mode = strcmp(value, "true") == 0;
        else if (strcmp(key, "search_highlight") == 0) config->search_highlight = strcmp(value, "true") == 0;
        else if (strcmp(key, "case_sensitive") == 0) config->case_sensitive = strcmp(value, "true") == 0;
        else if (strcmp(key, "show_status_bar") == 0) config->show_status_bar = strcmp(value, "true") == 0;
        else if (strcmp(key, "backup_interval") == 0) config->backup_interval = atoi(value);
        else if (strcmp(key, "create_backup") == 0) config->create_backup = strcmp(value, "true") == 0;
    }
    
    fclose(file);
    return 0;
}

int config_save(Config* config, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return -1;
    
    fprintf(file, "# Pappier Configuration File\n");
    fprintf(file, "# Generated automatically\n\n");
    
    fprintf(file, "tab_size = %d\n", config->tab_size);
    fprintf(file, "auto_indent = %s\n", config->auto_indent ? "true" : "false");
    fprintf(file, "show_line_numbers = %s\n", config->show_line_numbers ? "true" : "false");
    fprintf(file, "highlight_current_line = %s\n", config->highlight_current_line ? "true" : "false");
    fprintf(file, "wrap_lines = %s\n", config->wrap_lines ? "true" : "false");
    fprintf(file, "scroll_off = %d\n", config->scroll_off);
    fprintf(file, "theme = %s\n", config->theme_name);
    fprintf(file, "dark_mode = %s\n", config->dark_mode ? "true" : "false");
    fprintf(file, "search_highlight = %s\n", config->search_highlight ? "true" : "false");
    fprintf(file, "case_sensitive = %s\n", config->case_sensitive ? "true" : "false");
    fprintf(file, "show_status_bar = %s\n", config->show_status_bar ? "true" : "false");
    fprintf(file, "backup_interval = %d\n", config->backup_interval);
    fprintf(file, "create_backup = %s\n", config->create_backup ? "true" : "false");
    fprintf(file, "backup_dir = %s\n", config->backup_dir);
    
    fclose(file);
    return 0;
}

void config_apply(Config* config) {
    // Appliquer la configuration à l'éditeur
    // (sera appelée au démarrage)
}