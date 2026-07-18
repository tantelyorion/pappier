#ifndef THEME_H
#define THEME_H

#include <stdint.h>

// Palette de couleurs Vezo pour Pappier
typedef struct {
    // Interface
    uint32_t primary;      // #2F80FF
    uint32_t accent;       // #00D4FF
    uint32_t bg_primary;   // #080A0F
    uint32_t bg_surface;   // #161B22
    uint32_t text_primary; // #F5F7FA
    uint32_t text_secondary;// #8B95A9
    uint32_t border;       // #2D333B
    uint32_t hover;        // #1C2430
    
    // Syntaxe
    uint32_t keyword;      // #2F80FF
    uint32_t string;       // #00D4FF
    uint32_t comment;      // #6A737D
    uint32_t number;       // #F5A623
    uint32_t function;     // #7B61FF
    uint32_t variable;     // #F5F7FA
    uint32_t operator;     // #FF6B6B
    uint32_t type;         // #4ECB71
    
    // Cursor et sélection
    uint32_t cursor;       // #2F80FF
    uint32_t selection;    // #2F80FF33 (20% opacity)
    uint32_t line_highlight; // #161B2255
} Theme;

// Fonctions
void theme_init(Theme* theme);
void theme_apply_to_terminal(const Theme* theme);
const char* theme_get_color_code(uint32_t color);
uint32_t theme_hex_to_rgb(uint32_t hex, uint8_t* r, uint8_t* g, uint8_t* b);

#endif