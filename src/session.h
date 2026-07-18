#ifndef SESSION_H
#define SESSION_H

#include "editor.h"
#include "project.h"

typedef struct {
    char** files;
    int file_count;
    char* current_file;
    int cursor_line;
    int cursor_col;
    char* project_path;
    char* theme_name;
    int split_config;           // Configuration des splits
    time_t last_session;
} Session;

typedef struct {
    Session* sessions;
    int count;
    char* sessions_dir;
    int auto_save_interval;
} SessionManager;

SessionManager* session_init(void);
void session_free(SessionManager* sm);
void session_save(SessionManager* sm, Editor* editor, ProjectBrowser* project, const char* name);
int session_load(SessionManager* sm, Editor* editor, ProjectBrowser* project, const char* name);
void session_auto_save(SessionManager* sm, Editor* editor, ProjectBrowser* project);
void session_list(SessionManager* sm);
void session_delete(SessionManager* sm, const char* name);
void session_restore_last(SessionManager* sm, Editor* editor, ProjectBrowser* project);
void session_render(SessionManager* sm, int x, int y, int width, int height);

#endif