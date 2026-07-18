#include "theme_transparent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ThemeTransparent* theme_transparent_init(void) {
    ThemeTransparent* tt = malloc(sizeof(ThemeTransparent));
    
    // Initialiser le thème de base
    theme_init(&tt->base);
    
    // Paramètres de transparence
    tt->transparency = 230;      // Presque opaque
    tt->blur_effect = 1;         // Activé par défaut
    tt->glass_effect = 1;        // Effet verre
    tt->glass_opacity = 0.85;    // 85% d'opacité
    tt->background_image = NULL;
    tt->show_wallpaper = 1;
    
    return tt;
}

void theme_transparent_free(ThemeTransparent* tt) {
    if (!tt) return;
    if (tt->background_image) free(tt->background_image);
    free(tt);
}

void theme_transparent_apply(ThemeTransparent* tt) {
    // Appliquer le thème de base
    theme_apply_to_terminal(&tt->base);
    
    // Appliquer les effets de transparence
    #ifdef MIKEA_OS
    mikea_theme_apply_transparency(tt);
    #else
    // Simuler la transparence dans ncurses avec des codes ANSI
    printf("\033[38;2;%d;%d;%dm", 
           (tt->base.bg_primary >> 16) & 0xFF,
           (tt->base.bg_primary >> 8) & 0xFF,
           tt->base.bg_primary & 0xFF);
    #endif
}

void theme_transparent_set_opacity(ThemeTransparent* tt, int alpha) {
    if (alpha < 0) alpha = 0;
    if (alpha > 255) alpha = 255;
    tt->transparency = alpha;
}

void theme_transparent_set_blur(ThemeTransparent* tt, int enabled) {
    tt->blur_effect = enabled;
}

void theme_transparent_set_glass(ThemeTransparent* tt, float opacity) {
    if (opacity < 0) opacity = 0;
    if (opacity > 1) opacity = 1;
    tt->glass_opacity = opacity;
}

void theme_transparent_render_preview(ThemeTransparent* tt, int x, int y, int width, int height) {
    if (!tt) return;
    
    // Rendre un aperçu du thème avec transparence
    attron(A_BOLD);
    mvprintw(y, x, "🎨 Aperçu du thème transparent");
    attroff(A_BOLD);
    
    int line = y + 2;
    
    // Fond avec opacité
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < width - 10; j++) {
            mvaddch(line + i, x + 5 + j, ' ');
        }
    }
    
    // Informations
    mvprintw(line + 1, x + 10, "Opacité: %d%%", (tt->transparency * 100) / 255);
    mvprintw(line + 2, x + 10, "Flou: %s", tt->blur_effect ? "✅ Activé" : "❌ Désactivé");
    mvprintw(line + 3, x + 10, "Effet verre: %s (%.0f%%)", 
             tt->glass_effect ? "✅" : "❌", tt->glass_opacity * 100);
    
    // Indicateur de transparence
    int bar_width = 20;
    int filled = (tt->transparency * bar_width) / 255;
    mvprintw(line + 5, x + 10, "Transparence: [");
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            attron(COLOR_PAIR(2));
            mvaddch(line + 5, x + 24 + i, '=');
            attroff(COLOR_PAIR(2));
        } else {
            mvaddch(line + 5, x + 24 + i, ' ');
        }
    }
    mvprintw(line + 5, x + 24 + bar_width, "]");
}

#ifdef MIKEA_OS
void mikea_theme_apply_transparency(ThemeTransparent* tt) {
    // Appel à l'API Mikea OS pour la transparence
    // mikea_window_set_opacity(tt->transparency);
    // mikea_window_enable_blur(tt->blur_effect);
    // mikea_window_set_glass_effect(tt->glass_effect, tt->glass_opacity);
}

void mikea_theme_set_wallpaper(ThemeTransparent* tt, const char* wallpaper_path) {
    if (tt->background_image) free(tt->background_image);
    tt->background_image = strdup(wallpaper_path);
    tt->show_wallpaper = 1;
}

void mikea_theme_enable_acrylic(ThemeTransparent* tt) {
    tt->glass_effect = 1;
    tt->blur_effect = 1;
    tt->transparency = 200;
    tt->glass_opacity = 0.7;
}
#endif