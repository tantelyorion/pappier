#include "session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

SessionManager* session_init(void) {
    SessionManager* sm = malloc(sizeof(SessionManager));
    sm->sessions = NULL;
    sm->count = 0;
    sm->sessions_dir = strdup("~/.pappier/sessions");
    sm->auto_save_interval = 60; // 60 secondes
    
    // Créer le répertoire des sessions
    char mkdir_cmd[256];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", sm->sessions_dir);
    system(mkdir_cmd);
    
    return sm;
}

void session_free(SessionManager* sm) {
    if (!sm) return;
    
    for (int i = 0; i < sm->count; i++) {
        Session* s = &sm->sessions[i];
        if (s->files) {
            for (int j = 0; j < s->file_count; j++) {
                free(s->files[j]);
            }
            free(s->files);
        }
        if (s->current_file) free(s->current_file);
        if (s->project_path) free(s->project_path);
        if (s->theme_name) free(s->theme_name);
    }
    if (sm->sessions) free(sm->sessions);
    if (sm->sessions_dir) free(sm->sessions_dir);
    free(sm);
}

void session_save(SessionManager* sm, Editor* editor, ProjectBrowser* project, const char* name) {
    Session session;
    memset(&session, 0, sizeof(Session));
    
    // Sauvegarder les fichiers ouverts
    session.file_count = 1; // Simulé - dans la vraie implémentation, on aurait plus de fichiers
    session.files = malloc(sizeof(char*) * session.file_count);
    if (editor->filename) {
        session.files[0] = strdup(editor->filename);
    } else {
        session.files[0] = strdup("[Nouveau fichier]");
    }
    session.current_file = strdup(editor->filename ? editor->filename : "");
    session.cursor_line = editor->cursor_y;
    session.cursor_col = editor->cursor_x;
    
    if (project && project->project_path) {
        session.project_path = strdup(project->project_path);
    } else {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd))) {
            session.project_path = strdup(cwd);
        } else {
            session.project_path = strdup(".");
        }
    }
    
    session.theme_name = strdup(editor->theme.theme_name ? editor->theme.theme_name : "vezo-dark");
    session.split_config = 0;
    session.last_session = time(NULL);
    
    // Écrire dans un fichier
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s.session", sm->sessions_dir, name);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        // Libérer la mémoire
        for (int i = 0; i < session.file_count; i++) free(session.files[i]);
        free(session.files);
        free(session.current_file);
        free(session.project_path);
        free(session.theme_name);
        return;
    }
    
    fprintf(file, "# Pappier Session File\n");
    fprintf(file, "# Generated: %s", ctime(&session.last_session));
    fprintf(file, "\n");
    fprintf(file, "session_name = %s\n", name);
    fprintf(file, "session_time = %ld\n", session.last_session);
    fprintf(file, "\n");
    fprintf(file, "file_count = %d\n", session.file_count);
    for (int i = 0; i < session.file_count; i++) {
        fprintf(file, "file_%d = %s\n", i, session.files[i]);
    }
    fprintf(file, "current_file = %s\n", session.current_file);
    fprintf(file, "cursor_line = %d\n", session.cursor_line);
    fprintf(file, "cursor_col = %d\n", session.cursor_col);
    fprintf(file, "project_path = %s\n", session.project_path);
    fprintf(file, "theme = %s\n", session.theme_name);
    fprintf(file, "split_config = %d\n", session.split_config);
    
    fclose(file);
    
    // Ajouter à la liste des sessions
    sm->sessions = realloc(sm->sessions, sizeof(Session) * (sm->count + 1));
    sm->sessions[sm->count] = session;
    sm->count++;
    
    // Libérer la mémoire locale
    for (int i = 0; i < session.file_count; i++) free(session.files[i]);
    free(session.files);
}

int session_load(SessionManager* sm, Editor* editor, ProjectBrowser* project, const char* name) {
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s.session", sm->sessions_dir, name);
    
    FILE* file = fopen(filename, "r");
    if (!file) return -1;
    
    Session session;
    memset(&session, 0, sizeof(Session));
    
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        
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
        
        if (strcmp(key, "file_count") == 0) {
            session.file_count = atoi(value);
            session.files = malloc(sizeof(char*) * session.file_count);
        } else if (strncmp(key, "file_", 5) == 0) {
            int index = atoi(key + 5);
            if (index < session.file_count) {
                session.files[index] = strdup(value);
            }
        } else if (strcmp(key, "current_file") == 0) {
            session.current_file = strdup(value);
        } else if (strcmp(key, "cursor_line") == 0) {
            session.cursor_line = atoi(value);
        } else if (strcmp(key, "cursor_col") == 0) {
            session.cursor_col = atoi(value);
        } else if (strcmp(key, "project_path") == 0) {
            session.project_path = strdup(value);
        } else if (strcmp(key, "theme") == 0) {
            session.theme_name = strdup(value);
        } else if (strcmp(key, "split_config") == 0) {
            session.split_config = atoi(value);
        }
    }
    
    fclose(file);
    
    // Appliquer la session
    if (session.file_count > 0 && session.files[0]) {
        editor_load_file(editor, session.files[0]);
    }
    
    if (session.current_file) {
        // Charger le fichier actuel
    }
    
    editor->cursor_y = session.cursor_line;
    editor->cursor_x = session.cursor_col;
    
    if (project && session.project_path) {
        // Charger le projet
    }
    
    // Libérer la mémoire
    for (int i = 0; i < session.file_count; i++) {
        if (session.files[i]) free(session.files[i]);
    }
    if (session.files) free(session.files);
    if (session.current_file) free(session.current_file);
    if (session.project_path) free(session.project_path);
    if (session.theme_name) free(session.theme_name);
    
    return 0;
}

void session_auto_save(SessionManager* sm, Editor* editor, ProjectBrowser* project) {
    static time_t last_auto_save = 0;
    time_t now = time(NULL);
    
    if (difftime(now, last_auto_save) >= sm->auto_save_interval) {
        session_save(sm, editor, project, "autosave");
        last_auto_save = now;
    }
}

void session_list(SessionManager* sm) {
    DIR* dir = opendir(sm->sessions_dir);
    if (!dir) return;
    
    struct dirent* entry;
    printf("\n📂 Sessions disponibles:\n");
    int i = 1;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".session")) {
            char* name = strdup(entry->d_name);
            char* dot = strrchr(name, '.');
            if (dot) *dot = '\0';
            printf("  %d. %s\n", i++, name);
            free(name);
        }
    }
    closedir(dir);
}

void session_delete(SessionManager* sm, const char* name) {
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/%s.session", sm->sessions_dir, name);
    remove(filename);
}

void session_restore_last(SessionManager* sm, Editor* editor, ProjectBrowser* project) {
    session_load(sm, editor, project, "autosave");
}

void session_render(SessionManager* sm, int x, int y, int width, int height) {
    if (!sm) return;
    
    attron(A_BOLD);
    mvprintw(y, x, "💾 Sessions:");
    attroff(A_BOLD);
    
    DIR* dir = opendir(sm->sessions_dir);
    if (!dir) {
        mvprintw(y + 1, x + 2, "Aucune session trouvée");
        return;
    }
    
    int line = y + 1;
    struct dirent* entry;
    int count = 0;
    
    while ((entry = readdir(dir)) != NULL && line < y + height - 2) {
        if (strstr(entry->d_name, ".session")) {
            char* name = strdup(entry->d_name);
            char* dot = strrchr(name, '.');
            if (dot) *dot = '\0';
            
            // Obtenir la date
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", sm->sessions_dir, entry->d_name);
            struct stat st;
            if (stat(filepath, &st) == 0) {
                char time_str[64];
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", localtime(&st.st_mtime));
                mvprintw(line, x + 2, "  %-20s  %s", name, time_str);
            } else {
                mvprintw(line, x + 2, "  %s", name);
            }
            line++;
            free(name);
            count++;
        }
    }
    closedir(dir);
}