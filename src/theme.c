#include "theme.h"
#include <stdio.h>
#include <string.h>

void theme_init(Theme* theme) {
    // Interface
    theme->primary = 0x2F80FF;
    theme->accent = 0x00D4FF;
    theme->bg_primary = 0x080A0F;
    theme->bg_surface = 0x161B22;
    theme->text_primary = 0xF5F7FA;
    theme->text_secondary = 0x8B95A9;
    theme->border = 0x2D333B;
    theme->hover = 0x1C2430;
    
    // Syntaxe
    theme->keyword = 0x2F80FF;
    theme->string = 0x00D4FF;
    theme->comment = 0x6A737D;
    theme->number = 0xF5A623;
    theme->function = 0x7B61FF;
    theme->variable = 0xF5F7FA;
    theme->operator = 0xFF6B6B;
    theme->type = 0x4ECB71;
    
    // Cursor et sélection
    theme->cursor = 0x2F80FF;
    theme->selection = 0x2F80FF33;
    theme->line_highlight = 0x161B2255;
}

uint32_t theme_hex_to_rgb(uint32_t hex, uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = (hex >> 16) & 0xFF;
    *g = (hex >> 8) & 0xFF;
    *b = hex & 0xFF;
    return hex;
}

// ANSI color codes pour terminal
const char* theme_get_color_code(uint32_t color) {
    uint8_t r, g, b;
    theme_hex_to_rgb(color, &r, &g, &b);
    
    static char buffer[32];
    snprintf(buffer, sizeof(buffer), "\033[38;2;%d;%d;%dm", r, g, b);
    return buffer;
}