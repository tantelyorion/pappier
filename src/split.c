#include "split.h"
#include <stdlib.h>
#include <string.h>

SplitPane* split_create(Editor* editor) {
    SplitPane* pane = malloc(sizeof(SplitPane));
    pane->editor = editor;
    pane->left = NULL;
    pane->right = NULL;
    pane->parent = NULL;
    pane->type = SPLIT_VERTICAL;
    pane->ratio = 0.5;
    pane->active = 1;
    return pane;
}

void split_free(SplitPane* pane) {
    if (!pane) return;
    if (pane->left) split_free(pane->left);
    if (pane->right) split_free(pane->right);
    if (pane->editor) editor_free(pane->editor);
    free(pane);
}

void split_vertical(SplitPane* pane) {
    if (!pane) return;
    
    // Créer deux nouveaux panneaux
    SplitPane* left = split_create(editor_init());
    SplitPane* right = split_create(editor_init());
    
    left->parent = pane;
    right->parent = pane;
    left->active = 1;
    right->active = 0;
    
    // Copier le contenu de l'éditeur actuel
    // (implémentation simplifiée)
    
    pane->left = left;
    pane->right = right;
    pane->type = SPLIT_VERTICAL;
    pane->active = 0;
}

void split_horizontal(SplitPane* pane) {
    if (!pane) return;
    
    SplitPane* top = split_create(editor_init());
    SplitPane* bottom = split_create(editor_init());
    
    top->parent = pane;
    bottom->parent = pane;
    top->active = 1;
    bottom->active = 0;
    
    pane->left = top;
    pane->right = bottom;
    pane->type = SPLIT_HORIZONTAL;
    pane->active = 0;
}

void split_close(SplitPane* pane, SplitPane* target) {
    if (!pane || !target) return;
    
    if (pane->left == target) {
        split_free(pane->left);
        pane->left = NULL;
        if (pane->right) {
            pane->right->active = 1;
            if (pane->parent) {
                // Remplacer ce panneau par le panneau restant
                if (pane->parent->left == pane) {
                    pane->parent->left = pane->right;
                    pane->right->parent = pane->parent;
                } else if (pane->parent->right == pane) {
                    pane->parent->right = pane->right;
                    pane->right->parent = pane->parent;
                }
            }
        }
    } else if (pane->right == target) {
        split_free(pane->right);
        pane->right = NULL;
        if (pane->left) {
            pane->left->active = 1;
            if (pane->parent) {
                if (pane->parent->left == pane) {
                    pane->parent->left = pane->left;
                    pane->left->parent = pane->parent;
                } else if (pane->parent->right == pane) {
                    pane->parent->right = pane->left;
                    pane->left->parent = pane->parent;
                }
            }
        }
    }
}

static void find_next_active(SplitPane* pane, SplitPane** found) {
    if (!pane) return;
    if (pane->active) {
        *found = pane;
        return;
    }
    find_next_active(pane->left, found);
    find_next_active(pane->right, found);
}

void split_focus_next(SplitPane* pane) {
    // Parcourir l'arbre pour trouver le prochain panneau actif
    SplitPane* current = split_get_active(pane);
    if (!current) return;
    
    // Désactiver le panneau actuel
    current->active = 0;
    
    // Trouver le prochain panneau (parcours simple)
    SplitPane* next = NULL;
    if (current->parent) {
        if (current->parent->left == current && current->parent->right) {
            next = current->parent->right;
        } else if (current->parent->right == current && current->parent->left) {
            next = current->parent->left;
        }
    }
    
    if (next) {
        next->active = 1;
    }
}

void split_focus_prev(SplitPane* pane) {
    // Similaire à focus_next mais inverse
    split_focus_next(pane);
}

void split_resize(SplitPane* pane, float ratio) {
    if (ratio < 0.1) ratio = 0.1;
    if (ratio > 0.9) ratio = 0.9;
    pane->ratio = ratio;
}

void split_render_all(SplitPane* pane, int x, int y, int width, int height) {
    if (!pane) return;
    
    if (!pane->left && !pane->right) {
        // Panneau simple
        if (pane->editor) {
            pane->editor->screen_width = width - 2;
            pane->editor->screen_height = height - 2;
            
            // Afficher la bordure si actif
            if (pane->active) {
                attron(A_REVERSE);
            }
            mvhline(y, x, ACS_HLINE, width);
            mvhline(y + height - 1, x, ACS_HLINE, width);
            mvvline(y, x, ACS_VLINE, height);
            mvvline(y, x + width - 1, ACS_VLINE, height);
            mvaddch(y, x, ACS_ULCORNER);
            mvaddch(y, x + width - 1, ACS_URCORNER);
            mvaddch(y + height - 1, x, ACS_LLCORNER);
            mvaddch(y + height - 1, x + width - 1, ACS_LRCORNER);
            if (pane->active) {
                attroff(A_REVERSE);
            }
            
            // Rendre l'éditeur dans la zone
            editor_render(pane->editor);
        }
        return;
    }
    
    // Panneau divisé
    if (pane->type == SPLIT_VERTICAL) {
        int split_x = (int)(width * pane->ratio);
        
        if (pane->left) {
            split_render_all(pane->left, x, y, split_x, height);
        }
        if (pane->right) {
            split_render_all(pane->right, x + split_x, y, width - split_x, height);
        }
        
        // Ligne de séparation
        mvvline(y + 1, x + split_x, ACS_VLINE, height - 2);
    } else {
        int split_y = (int)(height * pane->ratio);
        
        if (pane->left) {
            split_render_all(pane->left, x, y, width, split_y);
        }
        if (pane->right) {
            split_render_all(pane->right, x, y + split_y, width, height - split_y);
        }
        
        // Ligne de séparation
        mvhline(y + split_y, x + 1, ACS_HLINE, width - 2);
    }
}

SplitPane* split_get_active(SplitPane* pane) {
    if (!pane) return NULL;
    if (pane->active) return pane;
    
    SplitPane* found = NULL;
    if (pane->left) found = split_get_active(pane->left);
    if (!found && pane->right) found = split_get_active(pane->right);
    return found;
}