#ifndef MACROS_H
#define MACROS_H

#include "editor.h"

typedef struct MacroAction {
    int key;
    char* description;
    struct MacroAction* next;
} MacroAction;

typedef struct {
    char* name;
    MacroAction* actions;
    int action_count;
    int is_recording;
    int is_playing;
    struct Macro* next;
} Macro;

typedef struct {
    Macro* macros;
    int count;
    Macro* current_recording;
    Macro* current_playing;
    int repeat_count;
} MacroManager;

MacroManager* macros_init(void);
void macros_free(MacroManager* mm);
void macros_start_recording(MacroManager* mm, const char* name);
void macros_stop_recording(MacroManager* mm);
void macros_record_key(MacroManager* mm, int key);
void macros_play(MacroManager* mm, Editor* editor, const char* name);
void macros_play_repeat(MacroManager* mm, Editor* editor, const char* name, int times);
void macros_delete(MacroManager* mm, const char* name);
void macros_list(MacroManager* mm);
void macros_save(MacroManager* mm, const char* filename);
void macros_load(MacroManager* mm, const char* filename);
void macros_render(MacroManager* mm, int x, int y, int width, int height);

#endif