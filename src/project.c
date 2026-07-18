#include "project.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

static int is_hidden(const char* name) {
    return name[0] == '.';
}

static int is_ignored(const char* name) {
    const char* ignored[] = {".git", "node_modules", "target", "build", "dist", "obj", "bin", NULL};
    for (int i = 0; ignored[i]; i++) {
        if (strcmp(name, ignored[i]) == 0) return 1;
    }
    return 0;
}

FileNode* file_node_create(const char* name, const char* path, int is_directory) {
    FileNode* node = malloc(sizeof(FileNode));
    node->name = strdup(name);
    node->path = strdup(path);
    node->is_directory = is_directory;
    node->is_expanded = 0;
    node->children = NULL;
    node->child_count = 0;
    node->parent = NULL;
    return node;
}

void file_node_free(FileNode* node) {
    if (!node) return;
    if (node->name) free(node->name);
    if (node->path) free(node->path);
    for (int i = 0; i < node->child_count; i++) {
        file_node_free(node->children[i]);
    }
    if (node->children) free(node->children);
    free(node);
}

ProjectBrowser* project_init(const char* path) {
    ProjectBrowser* pb = malloc(sizeof(ProjectBrowser));
    pb->project_path = strdup(path);
    pb->selected_index = 0;
    pb->scroll_offset = 0;
    pb->visible_count = 0;
    pb->expanded_paths = malloc(sizeof(char*) * 100);
    pb->expanded_count = 0;
    
    pb->root = file_node_create("/", path, 1);
    project_build_tree(pb, pb->root);
    
    return pb;
}

void project_free(ProjectBrowser* pb) {
    if (!pb) return;
    file_node_free(pb->root);
    if (pb->project_path) free(pb->project_path);
    for (int i = 0; i < pb->expanded_count; i++) {
        free(pb->expanded_paths[i]);
    }
    free(pb->expanded_paths);
    free(pb);
}

void project_build_tree(ProjectBrowser* pb, FileNode* node) {
    DIR* dir = opendir(node->path);
    if (!dir) return;
    
    struct dirent* entry;
    struct stat st;
    char full_path[1024];
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (is_hidden(entry->d_name)) continue;
        if (is_ignored(entry->d_name)) continue;
        
        snprintf(full_path, sizeof(full_path), "%s/%s", node->path, entry->d_name);
        stat(full_path, &st);
        
        int is_dir = S_ISDIR(st.st_mode);
        FileNode* child = file_node_create(entry->d_name, full_path, is_dir);
        child->parent = node;
        
        node->children = realloc(node->children, sizeof(FileNode*) * (node->child_count + 1));
        node->children[node->child_count++] = child;
        
        if (is_dir) {
            // Vérifier si le dossier doit être expansé
            for (int i = 0; i < pb->expanded_count; i++) {
                if (strcmp(pb->expanded_paths[i], full_path) == 0) {
                    child->is_expanded = 1;
                    project_build_tree(pb, child);
                    break;
                }
            }
        }
    }
    closedir(dir);
}

void project_refresh(ProjectBrowser* pb, const char* path) {
    // Nettoyer l'ancien arbre
    file_node_free(pb->root);
    
    // Reconstruire
    if (path) {
        if (pb->project_path) free(pb->project_path);
        pb->project_path = strdup(path);
    }
    pb->root = file_node_create("/", pb->project_path, 1);
    project_build_tree(pb, pb->root);
}

void project_toggle_node(ProjectBrowser* pb, FileNode* node) {
    if (!node || !node->is_directory) return;
    
    node->is_expanded = !node->is_expanded;
    
    if (node->is_expanded) {
        // Ajouter aux chemins expansés
        pb->expanded_paths = realloc(pb->expanded_paths, sizeof(char*) * (pb->expanded_count + 1));
        pb->expanded_paths[pb->expanded_count++] = strdup(node->path);
        project_build_tree(pb, node);
    } else {
        // Retirer des chemins expansés
        for (int i = 0; i < pb->expanded_count; i++) {
            if (strcmp(pb->expanded_paths[i], node->path) == 0) {
                free(pb->expanded_paths[i]);
                for (int j = i; j < pb->expanded_count - 1; j++) {
                    pb->expanded_paths[j] = pb->expanded_paths[j + 1];
                }
                pb->expanded_count--;
                break;
            }
        }
        // Supprimer les enfants
        for (int i = 0; i < node->child_count; i++) {
            file_node_free(node->children[i]);
        }
        node->children = NULL;
        node->child_count = 0;
    }
}

FileNode* project_get_selected(ProjectBrowser* pb) {
    // Parcourir l'arbre pour trouver le n-ième élément visible
    // Implémentation simplifiée
    return pb->root;
}

void project_select_next(ProjectBrowser* pb) {
    pb->selected_index++;
}

void project_select_prev(ProjectBrowser* pb) {
    if (pb->selected_index > 0) pb->selected_index--;
}

void project_open_selected(ProjectBrowser* pb, Editor* editor) {
    // Ouvrir le fichier sélectionné
    FileNode* node = project_get_selected(pb);
    if (node && !node->is_directory) {
        editor_load_file(editor, node->path);
    }
}

static void render_node(FileNode* node, int depth, int* y, int selected, int scroll, int max_y) {
    if (*y < scroll) {
        (*y)++;
        return;
    }
    if (*y >= scroll + max_y) return;
    
    char indent[64];
    memset(indent, ' ', depth * 2);
    indent[depth * 2] = '\0';
    
    const char* icon = node->is_directory ? "📁 " : "📄 ";
    if (node->is_directory && node->is_expanded) icon = "📂 ";
    
    char line[256];
    snprintf(line, sizeof(line), "%s%s%s", indent, icon, node->name);
    
    if (*y - scroll == selected) {
        attron(A_REVERSE);
    }
    mvprintw(*y - scroll + 1, 2, "%-*s", 50, line);
    if (*y - scroll == selected) {
        attroff(A_REVERSE);
    }
    (*y)++;
    
    if (node->is_directory && node->is_expanded) {
        for (int i = 0; i < node->child_count; i++) {
            render_node(node->children[i], depth + 1, y, selected, scroll, max_y);
        }
    }
}

void project_render(ProjectBrowser* pb, int x, int y, int width, int height) {
    if (!pb || !pb->root) return;
    
    // Afficher l'en-tête
    attron(A_BOLD);
    mvprintw(y, x, "📁 Projet: %s", pb->project_path ? strrchr(pb->project_path, '/') + 1 : "");
    attroff(A_BOLD);
    
    int line_y = 0;
    int max_visible = height - 3;
    
    render_node(pb->root, 0, &line_y, pb->selected_index, pb->scroll_offset, max_visible);
}