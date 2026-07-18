#ifndef PROJECT_H
#define PROJECT_H

#include "editor.h"
#include <dirent.h>
#include <sys/stat.h>

typedef struct FileNode {
    char* name;
    char* path;
    int is_directory;
    int is_expanded;
    struct FileNode** children;
    int child_count;
    struct FileNode* parent;
} FileNode;

typedef struct {
    FileNode* root;
    char* project_path;
    int selected_index;
    int scroll_offset;
    int visible_count;
    char** expanded_paths;
    int expanded_count;
} ProjectBrowser;

ProjectBrowser* project_init(const char* path);
void project_free(ProjectBrowser* pb);
void project_refresh(ProjectBrowser* pb, const char* path);
void project_build_tree(ProjectBrowser* pb, FileNode* node);
void project_toggle_node(ProjectBrowser* pb, FileNode* node);
FileNode* project_get_selected(ProjectBrowser* pb);
void project_select_next(ProjectBrowser* pb);
void project_select_prev(ProjectBrowser* pb);
void project_open_selected(ProjectBrowser* pb, Editor* editor);
void project_render(ProjectBrowser* pb, int x, int y, int width, int height);

#endif