#ifndef THEME_TRANSPARENT_H
#define THEME_TRANSPARENT_H

#include "theme.h"

typedef struct {
    Theme base;
    int transparency;           // 0-255 (0 = totalement transparent, 255 = opaque)
    int blur_effect;            // 0 = désactivé, 1 = activé
    int glass_effect;           // Effet verre
    float glass_opacity;        // 0.0 - 1.0
    char* background_image;     // Chemin vers l'image de fond
    int show_wallpaper;         // Afficher le fond d'écran du système
} ThemeTransparent;

ThemeTransparent* theme_transparent_init(void);
void theme_transparent_free(ThemeTransparent* tt);
void theme_transparent_apply(ThemeTransparent* tt);
void theme_transparent_set_opacity(ThemeTransparent* tt, int alpha);
void theme_transparent_set_blur(ThemeTransparent* tt, int enabled);
void theme_transparent_set_glass(ThemeTransparent* tt, float opacity);
void theme_transparent_render_preview(ThemeTransparent* tt, int x, int y, int width, int height);

// Fonctions Mikea OS spécifiques
#ifdef MIKEA_OS
void mikea_theme_apply_transparency(ThemeTransparent* tt);
void mikea_theme_set_wallpaper(ThemeTransparent* tt, const char* wallpaper_path);
void mikea_theme_enable_acrylic(ThemeTransparent* tt);
#endif

#endif