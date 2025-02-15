#include "theme_manager.h"
#include "raygui.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include "storage.h"
#endif

ThemeManager themeManager = {0};

void InitThemeManager(void) {
    // Light Theme
    themeManager.themes[0] = (Theme){
        .background = (Color){245, 245, 245, 255},  // Light gray
        .text = BLACK,
        .primary = (Color){70, 130, 180, 255},      // Steel blue
        .secondary = (Color){176, 196, 222, 255},   // Light steel blue
        .accent = (Color){65, 105, 225, 255},       // Royal blue
        .name = "Light"
    };

    // Dark Theme
    themeManager.themes[1] = (Theme){
        .background = (Color){40, 44, 52, 255},     // Dark gray
        .text = WHITE,
        .primary = (Color){86, 156, 214, 255},      // Blue
        .secondary = (Color){78, 86, 102, 255},     // Slate
        .accent = (Color){197, 134, 192, 255},      // Purple
        .name = "Dark"
    };

    // Sunset Theme
    themeManager.themes[2] = (Theme){
        .background = (Color){255, 243, 224, 255},  // Light orange
        .text = (Color){51, 51, 51, 255},          // Dark gray
        .primary = (Color){255, 87, 34, 255},      // Deep orange
        .secondary = (Color){255, 183, 77, 255},   // Orange
        .accent = (Color){233, 30, 99, 255},       // Pink
        .name = "Sunset"
    };

    themeManager.currentTheme = 0;
    themeManager.dropdownActive = false;
    LoadThemePreference();
}

void UpdateThemeManager(void) {
    const int dropdownWidth = 120;
    const int dropdownHeight = 30;
    themeManager.dropdownBounds = (Rectangle){
        GetScreenWidth() - dropdownWidth - 10,
        10,
        dropdownWidth,
        dropdownHeight
    };
}

void DrawThemeSelector(void) {
    static bool dropdownEditMode = false;
    static const char* themeNames[] = {"Light", "Dark", "Sunset"};
    
    if (GuiDropdownBox(themeManager.dropdownBounds, 
                      "Light;Dark;Sunset", 
                      &themeManager.currentTheme, 
                      dropdownEditMode)) {
        dropdownEditMode = !dropdownEditMode;
        if (!dropdownEditMode) {
            SaveThemePreference();
        }
    }
}

Theme GetCurrentTheme(void) {
    return themeManager.themes[themeManager.currentTheme];
}
void SaveThemePreference(void) {
#ifdef __EMSCRIPTEN__
    char buffer[64];
    sprintf(buffer, "%d", themeManager.currentTheme);
    js_saveToLocalStorage("themePreference", buffer);
#else
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/theme.dat", get_data_path());
    FILE *file = fopen(filepath, "wb");
    if (file) {
        fwrite(&themeManager.currentTheme, sizeof(int), 1, file);
        fclose(file);
    }
#endif
}

void LoadThemePreference(void) {
#ifdef __EMSCRIPTEN__
    char* data = js_loadFromLocalStorage("themePreference");
    if (data) {
        themeManager.currentTheme = atoi(data);
        free(data);
        // Ensure valid theme index
        if (themeManager.currentTheme < 0 || themeManager.currentTheme > 2) {
            themeManager.currentTheme = 0;
        }
    }
#else
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/theme.dat", get_data_path());
    FILE *file = fopen(filepath, "rb");
    if (file) {
        size_t read = fread(&themeManager.currentTheme, sizeof(int), 1, file);
        if (read != 1) themeManager.currentTheme = 0;
        fclose(file);
        // Ensure valid theme index
        if (themeManager.currentTheme < 0 || themeManager.currentTheme > 2) {
            themeManager.currentTheme = 0;
        }
    }
#endif
}