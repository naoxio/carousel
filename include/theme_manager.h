#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include "raylib.h"
#include "storage.h"
typedef struct {
    Color background;
    Color text;
    Color primary;
    Color secondary;
    Color accent;
    const char* name;
} Theme;

typedef struct {
    Theme themes[3];
    int currentTheme;
    bool dropdownActive;
    Rectangle dropdownBounds;
} ThemeManager;

extern ThemeManager themeManager;

void InitThemeManager(void);
void UpdateThemeManager(void);
void DrawThemeSelector(void);
Theme GetCurrentTheme(void);
void SaveThemePreference(void);
void LoadThemePreference(void);

#endif
