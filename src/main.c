#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h" 
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

#define MAX_OPTIONS 20
#define MAX_OPTION_LENGTH 50

typedef struct {
    char text[MAX_OPTION_LENGTH];
    Color color;
} Option;

typedef struct {
    Option options[MAX_OPTIONS];
    int count;
    float currentAngle;
    float targetAngle;
    bool isSpinning;
    float spinSpeed;
} Carousel;

static char globalInputText[MAX_OPTION_LENGTH] = {0};
static bool globalIsEditMode = false;
static int globalEditIndex = -1;
static Carousel* globalCarousel = NULL;
static float cursorBlinkTime = 0.0f;
static bool showCursor = true;

char* GetGlobalInputText(void) {
    return globalInputText;
}

int GetInputTextLength(void) {
    return MAX_OPTION_LENGTH;
}

// Function declarations
void SaveOptions(Carousel *carousel);
void LoadOptions(Carousel *carousel);
void AddOption(Carousel *carousel, const char *text);
void DeleteOption(Carousel *carousel, int index);
void DrawCarousel(Carousel *carousel);
Color GenerateRandomColor(void);
void UpdateCarouselSpin(Carousel *carousel);
float GetSectorAngle(int optionCount);
void AdjustToNearestSector(Carousel *carousel);

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <pwd.h>
#include <unistd.h>
#endif

// Helper function to get storage path/key
#ifndef __EMSCRIPTEN__
char* get_data_path() {
    static char path[1024];
    const char *home;
    
    // Try XDG_DATA_HOME first
    home = getenv("XDG_DATA_HOME");
    if (home != NULL) {
        snprintf(path, sizeof(path), "%s/carousel", home);
    } else {
        // Fall back to ~/.local/share
        home = getenv("HOME");
        if (home == NULL) {
            home = getpwuid(getuid())->pw_dir;
        }
        snprintf(path, sizeof(path), "%s/.local/share/carousel", home);
    }
    
    // Create directory if it doesn't exist
    mkdir(path, 0755);
    
    return path;
}
#endif

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
void emscripten_notify_key_pressed(int key);
void ProcessKeyPress(int key);
#endif

int main(void) {
    InitWindow(800, 600, "Carousel");
    SetTargetFPS(60);

    Carousel carousel = {0};
    carousel.currentAngle = -90.0f;
    carousel.isSpinning = false;
    
    LoadOptions(&carousel);

    // Set global carousel pointer
    globalCarousel = &carousel;

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointCircle(mousePos, (Vector2){400, 300}, 150)) {
                if (carousel.count > 0) {
                    carousel.isSpinning = true;
                    carousel.spinSpeed = 20.0f + (float)GetRandomValue(0, 10);
                    carousel.targetAngle = carousel.currentAngle + 
                        (float)GetRandomValue(720, 1440);
                }
            }
        }
        #ifndef __EMSCRIPTEN__
            // Desktop input handling
            int key = GetCharPressed();
            while (key > 0) {
                if ((strlen(globalInputText) < MAX_OPTION_LENGTH - 1) && 
                    (key >= 32) && (key <= 125)) {
                    globalInputText[strlen(globalInputText)] = (char)key;
                    globalInputText[strlen(globalInputText)] = '\0';
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = strlen(globalInputText);
                if (len > 0) globalInputText[len - 1] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER) && strlen(globalInputText) > 0) {
                if (globalIsEditMode && globalEditIndex >= 0) {
                    strcpy(carousel.options[globalEditIndex].text, globalInputText);
                    globalIsEditMode = false;
                    globalEditIndex = -1;
                } else {
                    AddOption(&carousel, globalInputText);
                }
                SaveOptions(&carousel);
                memset(globalInputText, 0, MAX_OPTION_LENGTH);
            }
        #endif

        UpdateCarouselSpin(&carousel);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        cursorBlinkTime += GetFrameTime();
        if (cursorBlinkTime >= 0.5f) {
            cursorBlinkTime = 0;
            showCursor = !showCursor;
        }
        DrawCarousel(&carousel);

        // Draw input field background
        DrawRectangle(10, 550, 780, 40, LIGHTGRAY);
        
        // Draw input text
        DrawText(globalInputText, 20, 560, 20, BLACK);
        
        // Draw blinking cursor
        if (showCursor) {
            int textWidth = MeasureText(globalInputText, 20);
            DrawRectangle(20 + textWidth, 560, 2, 20, BLACK);
        }

        DrawText(globalIsEditMode ? "Edit Mode" : "Add Option", 20, 520, 20, BLACK);

        for (int i = 0; i < carousel.count; i++) {
            DrawText(TextFormat("%d: %s", i + 1, carousel.options[i].text), 
                    20, 50 + i * 30, 20, carousel.options[i].color);
            
            Rectangle delButton = {220, 50 + i * 30, 60, 20};
            if (GuiButton(delButton, "Delete")) {
                DeleteOption(&carousel, i);
                SaveOptions(&carousel);
            }

            Rectangle editButton = {290, 50 + i * 30, 60, 20};
            if (GuiButton(editButton, "Edit")) {
                strcpy(globalInputText, carousel.options[i].text);
                globalIsEditMode = true;
                globalEditIndex = i;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
void emscripten_notify_key_pressed(int key) {
    ProcessKeyPress(key);
}

void ProcessKeyPress(int key) {
    char* inputText = GetGlobalInputText();
    int inputLength = GetInputTextLength();
    
    if (key == KEY_BACKSPACE) {
        int len = strlen(inputText);
        if (len > 0) inputText[len - 1] = '\0';
    }
    else if (key == KEY_ENTER) {
        if (strlen(inputText) > 0) {
            if (globalIsEditMode && globalEditIndex >= 0) {
                strcpy(globalCarousel->options[globalEditIndex].text, inputText);
                globalIsEditMode = false;
                globalEditIndex = -1;
            } else {
                AddOption(globalCarousel, inputText);
            }
            SaveOptions(globalCarousel);
            memset(inputText, 0, MAX_OPTION_LENGTH);
        }
    }
    else if ((key >= 32) && (key <= 125)) {
        if (strlen(inputText) < inputLength - 1) {
            int len = strlen(inputText);
            inputText[len] = (char)key;
            inputText[len + 1] = '\0';
        }
    }
}

#endif

void DrawCarousel(Carousel *carousel) {
    if (carousel->count == 0) return;

    float centerX = 400;
    float centerY = 300;
    float radius = 150;
    float sectorAngle = GetSectorAngle(carousel->count);

    // Draw sectors with 90-degree offset so sectors align with needle at top
    for (int i = 0; i < carousel->count; i++) {
        float startAngle = carousel->currentAngle + i * sectorAngle - 90;  // Offset by 90 degrees
        DrawCircleSector((Vector2){centerX, centerY}, radius, 
                        startAngle, startAngle + sectorAngle, 
                        32, carousel->options[i].color);
        
        // Calculate text position and rotation (also offset by 90 degrees)
        float textAngle = startAngle + sectorAngle / 2;
        float textX = centerX + cosf(textAngle * DEG2RAD) * (radius * 0.6f);
        float textY = centerY + sinf(textAngle * DEG2RAD) * (radius * 0.6f);
        
        Vector2 textPos = {textX, textY};
        int textWidth = MeasureText(carousel->options[i].text, 20);
        Vector2 textOrigin = {textWidth/2.0f, 10.0f};
        float rotation = textAngle;
        
        DrawTextPro(GetFontDefault(), carousel->options[i].text,
                   textPos, textOrigin, rotation, 20, 1, WHITE);
    }

    // Draw needle
    DrawTriangle(
        (Vector2){centerX, centerY - radius - 20},
        (Vector2){centerX - 10, centerY - radius},
        (Vector2){centerX + 10, centerY - radius},
        RED
    );
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
void js_saveToLocalStorage(const char* key, const char* value) {
    EM_ASM_({
        try {
            localStorage.setItem(UTF8ToString($0), UTF8ToString($1));
        } catch(e) {
            console.error('Save failed:', e);
        }
    }, key, value);
}

EMSCRIPTEN_KEEPALIVE
char* js_loadFromLocalStorage(const char* key) {
    return (char*)EM_ASM_INT({
        try {
            var value = localStorage.getItem(UTF8ToString($0)) || '';
            var length = lengthBytesUTF8(value) + 1;
            var buffer = _malloc(length);
            stringToUTF8(value, buffer, length);
            return buffer;
        } catch(e) {
            console.error('Load failed:', e);
            return 0;
        }
    }, key);
}
#endif

void SaveOptions(Carousel *carousel) {
#ifdef __EMSCRIPTEN__
    char buffer[4096] = {0};
    char *ptr = buffer;
    
    ptr += sprintf(ptr, "%d;", carousel->count);
    
    for (int i = 0; i < carousel->count; i++) {
        ptr += sprintf(ptr, "%s,%d,%d,%d;", 
            carousel->options[i].text,
            carousel->options[i].color.r,
            carousel->options[i].color.g,
            carousel->options[i].color.b
        );
    }
    
    js_saveToLocalStorage("carouselData", buffer);
#else
    // Native file save code remains the same
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/options.dat", get_data_path());
    
    FILE *file = fopen(filepath, "wb");
    if (file) {
        fwrite(&carousel->count, sizeof(int), 1, file);
        fwrite(carousel->options, sizeof(Option), carousel->count, file);
        fclose(file);
    }
#endif
}

void LoadOptions(Carousel *carousel) {
#ifdef __EMSCRIPTEN__
    char* data = js_loadFromLocalStorage("carouselData");
    
    if (!data) {
        carousel->count = 0;
        return;
    }
    
    char *token = strtok(data, ";");
    if (token) {
        carousel->count = atoi(token);
        
        int i = 0;
        while ((token = strtok(NULL, ";")) && i < carousel->count) {
            char *text = strtok(token, ",");
            char *r = strtok(NULL, ",");
            char *g = strtok(NULL, ",");
            char *b = strtok(NULL, ",");
            
            if (text && r && g && b) {
                strncpy(carousel->options[i].text, text, MAX_OPTION_LENGTH - 1);
                carousel->options[i].text[MAX_OPTION_LENGTH - 1] = '\0';
                carousel->options[i].color = (Color){
                    (unsigned char)atoi(r),
                    (unsigned char)atoi(g),
                    (unsigned char)atoi(b),
                    255
                };
                i++;
            }
        }
        carousel->count = i;
    }
    
    free(data);
#else
    // Native file load code remains the same
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/options.dat", get_data_path());
    
    FILE *file = fopen(filepath, "rb");
    if (file) {
        size_t read;
        read = fread(&carousel->count, sizeof(int), 1, file);
        if (read != 1) {
            carousel->count = 0;
        }
        read = fread(carousel->options, sizeof(Option), carousel->count, file);
        if (read != carousel->count) {
            carousel->count = 0;
        }
        fclose(file);
    } else {
        carousel->count = 0;
    }
#endif
}

void AddOption(Carousel *carousel, const char *text) {
    if (carousel->count < MAX_OPTIONS) {
        strcpy(carousel->options[carousel->count].text, text);
        carousel->options[carousel->count].color = GenerateRandomColor();
        carousel->count++;
    }
}

void DeleteOption(Carousel *carousel, int index) {
    if (index >= 0 && index < carousel->count) {
        for (int i = index; i < carousel->count - 1; i++) {
            carousel->options[i] = carousel->options[i + 1];
        }
        carousel->count--;
    }
}

Color GenerateRandomColor(void) {
    return (Color){
        GetRandomValue(50, 255),
        GetRandomValue(50, 255),
        GetRandomValue(50, 255),
        255
    };
}

float GetSectorAngle(int optionCount) {
    return 360.0f / optionCount;
}
void UpdateCarouselSpin(Carousel *carousel) {
    if (carousel->isSpinning) {
        if (carousel->spinSpeed > 1.0f) {
            // Normal spinning deceleration
            carousel->currentAngle += carousel->spinSpeed;
            carousel->spinSpeed *= 0.97f;
        } else {
            // We're going slow enough to start centering
            float sectorAngle = GetSectorAngle(carousel->count);
            float halfSectorAngle = sectorAngle / 2.0f;
            
            // Find current sector and its center
            float normalizedAngle = carousel->currentAngle + halfSectorAngle;
            int sector = floor(normalizedAngle / sectorAngle);
            float targetAngle = (sector * sectorAngle) - halfSectorAngle;
            
            // Calculate shortest path to target
            float angleDiff = targetAngle - carousel->currentAngle;
            if (angleDiff > 180.0f) angleDiff -= 360.0f;
            if (angleDiff < -180.0f) angleDiff += 360.0f;
            
            // Smoothly move towards center
            float moveSpeed = angleDiff * 0.1f;
            carousel->currentAngle += moveSpeed;
            
            // Stop when very close to center
            if (fabs(moveSpeed) < 0.01f) {
                carousel->isSpinning = false;
                carousel->currentAngle = targetAngle;
                carousel->spinSpeed = 0.0f;  // Reset spin speed when stopped
            }
        }

        // Keep angle within 360 degrees
        while (carousel->currentAngle >= 360.0f) {
            carousel->currentAngle -= 360.0f;
        }
        while (carousel->currentAngle < 0.0f) {
            carousel->currentAngle += 360.0f;
        }
    }
}
void AdjustToNearestSector(Carousel *carousel) {
    if (carousel->count == 0) return;

    float sectorAngle = GetSectorAngle(carousel->count);
    float halfSectorAngle = sectorAngle / 2.0f;
    
    // Normalize angle to nearest sector center
    float normalizedAngle = carousel->currentAngle + halfSectorAngle;
    int sector = floor(normalizedAngle / sectorAngle);
    carousel->currentAngle = (sector * sectorAngle) - halfSectorAngle;
    
    // Keep angle within proper range
    while (carousel->currentAngle >= 360.0f) {
        carousel->currentAngle -= 360.0f;
    }
    while (carousel->currentAngle < 0.0f) {
        carousel->currentAngle += 360.0f;
    }
    
    carousel->spinSpeed = 0.0f;
    carousel->isSpinning = false;
}