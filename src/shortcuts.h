#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include "editor.h"
#include "config.h"

typedef struct {
    char* key;
    char* action;
    void (*callback)(Editor*);
} Shortcut;

typedef struct {
    Shortcut* shortcuts;
    int count;
    int capacity;
} ShortcutManager;

ShortcutManager* shortcuts_init(void);
void shortcuts_free(ShortcutManager* sm);
void shortcuts_load(ShortcutManager* sm, Config* config);
void shortcuts_add(ShortcutManager* sm, const char* key, const char* action, void (*callback)(Editor*));
void shortcuts_execute(ShortcutManager* sm, Editor* editor, const char* key);

// Actions prédéfinies
void action_save(Editor* editor);
void action_open(Editor* editor);
void action_find(Editor* editor);
void action_replace(Editor* editor);
void action_undo(Editor* editor);
void action_redo(Editor* editor);
void action_select_all(Editor* editor);
void action_cut(Editor* editor);
void action_copy(Editor* editor);
void action_paste(Editor* editor);
void action_quit(Editor* editor);

#endif