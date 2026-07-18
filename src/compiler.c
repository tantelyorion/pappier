#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

CompilerConfig* compiler_init(void) {
    CompilerConfig* config = malloc(sizeof(CompilerConfig));
    config->compiler_path = strdup("gcc");
    config->flags = malloc(sizeof(char*) * 10);
    config->flag_count = 0;
    config->output_dir = strdup("./build");
    
    // Flags par défaut
    config->flags[config->flag_count++] = strdup("-Wall");
    config->flags[config->flag_count++] = strdup("-Wextra");
    config->flags[config->flag_count++] = strdup("-O2");
    
    return config;
}

void compiler_free(CompilerConfig* config) {
    if (!config) return;
    if (config->compiler_path) free(config->compiler_path);
    for (int i = 0; i < config->flag_count; i++) {
        free(config->flags[i]);
    }
    free(config->flags);
    if (config->output_dir) free(config->output_dir);
    free(config);
}

int compiler_detect_language(const char* filename) {
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    if (strcmp(ext, ".c") == 0) return 1;
    if (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cc") == 0) return 2;
    if (strcmp(ext, ".py") == 0) return 3;
    if (strcmp(ext, ".js") == 0) return 4;
    if (strcmp(ext, ".rs") == 0) return 5;
    
    return 0;
}

CompileResult* compiler_compile(CompilerConfig* config, const char* source_file) {
    CompileResult* result = malloc(sizeof(CompileResult));
    result->output = NULL;
    result->errors = NULL;
    result->exit_code = 0;
    result->compile_time = 0;
    result->success = 0;
    
    int lang = compiler_detect_language(source_file);
    if (lang == 0) {
        result->errors = strdup("Langage non supporté");
        return result;
    }
    
    // Construire la commande
    char cmd[2048];
    char output_file[1024];
    char* basename = strrchr(source_file, '/');
    if (basename) basename++; else basename = (char*)source_file;
    
    snprintf(output_file, sizeof(output_file), "%s/%s.out", 
             config->output_dir, basename);
    
    // Supprimer l'extension
    char* dot = strrchr(output_file, '.');
    if (dot) *dot = '\0';
    
    if (lang == 1) { // C
        snprintf(cmd, sizeof(cmd), "%s %s", config->compiler_path, source_file);
        for (int i = 0; i < config->flag_count; i++) {
            strcat(cmd, " ");
            strcat(cmd, config->flags[i]);
        }
        strcat(cmd, " -o ");
        strcat(cmd, output_file);
    } else if (lang == 2) { // C++
        snprintf(cmd, sizeof(cmd), "g++ %s", source_file);
        for (int i = 0; i < config->flag_count; i++) {
            strcat(cmd, " ");
            strcat(cmd, config->flags[i]);
        }
        strcat(cmd, " -o ");
        strcat(cmd, output_file);
    } else {
        result->errors = strdup("Compilation directe non supportée pour ce langage");
        return result;
    }
    
    // Créer le répertoire de sortie
    char mkdir_cmd[256];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", config->output_dir);
    system(mkdir_cmd);
    
    // Exécuter la compilation
    struct timeval start, end;
    gettimeofday(&start, NULL);
    
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        result->errors = strdup("Erreur d'exécution");
        return result;
    }
    
    char buffer[4096];
    char* errors = NULL;
    size_t total = 0;
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        size_t len = strlen(buffer);
        errors = realloc(errors, total + len + 1);
        strcpy(errors + total, buffer);
        total += len;
    }
    
    int status = pclose(pipe);
    
    gettimeofday(&end, NULL);
    result->compile_time = (end.tv_sec - start.tv_sec) * 1000 + 
                          (end.tv_usec - start.tv_usec) / 1000;
    
    result->exit_code = WEXITSTATUS(status);
    result->success = result->exit_code == 0;
    
    if (errors && strlen(errors) > 0) {
        result->errors = errors;
    } else {
        result->errors = NULL;
        free(errors);
    }
    
    if (result->success) {
        result->output = strdup(output_file);
    }
    
    return result;
}

void compiler_run(CompilerConfig* config, const char* executable) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./%s", executable);
    system(cmd);
}

void compiler_render_output(CompileResult* result, int x, int y, int width, int height) {
    if (!result) return;
    
    int line = y + 1;
    
    if (result->success) {
        attron(COLOR_PAIR(4)); // Vert
        mvprintw(line++, x, "✅ Compilation réussie");
        attroff(COLOR_PAIR(4));
        mvprintw(line++, x, "⏱️  Temps: %d ms", result->compile_time);
        mvprintw(line++, x, "📁 Sortie: %s", result->output ? result->output : "N/A");
    } else {
        attron(COLOR_PAIR(3)); // Rouge
        mvprintw(line++, x, "❌ Compilation échouée");
        attroff(COLOR_PAIR(3));
        mvprintw(line++, x, "Code d'erreur: %d", result->exit_code);
        
        if (result->errors) {
            line++;
            attron(COLOR_PAIR(3));
            mvprintw(line++, x, "Erreurs:");
            attroff(COLOR_PAIR(3));
            
            char* error_copy = strdup(result->errors);
            char* line_str = strtok(error_copy, "\n");
            while (line_str && line < y + height - 2) {
                mvprintw(line++, x, "  %s", line_str);
                line_str = strtok(NULL, "\n");
            }
            free(error_copy);
        }
    }
}