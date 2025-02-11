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

int main(void) {
    InitWindow(800, 600, "Carousel Options");
    SetTargetFPS(60);

    Carousel carousel = {0};
    carousel.currentAngle = -90.0f;
    carousel.isSpinning = false;
    
    LoadOptions(&carousel);

    char inputText[MAX_OPTION_LENGTH] = {0};
    bool isEditMode = false;
    int editIndex = -1;

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointCircle(mousePos, (Vector2){400, 300}, 150)) {
                if (carousel.count > 0) {  // Removed the !carousel.isSpinning check
                    carousel.isSpinning = true;
                    carousel.spinSpeed = 20.0f + (float)GetRandomValue(0, 10);
                    carousel.targetAngle = carousel.currentAngle + 
                        (float)GetRandomValue(720, 1440);
                }
            }
        }

        int key = GetCharPressed();
        while (key > 0) {
            if ((strlen(inputText) < MAX_OPTION_LENGTH - 1) && 
                (key >= 32) && (key <= 125)) {
                inputText[strlen(inputText)] = (char)key;
                inputText[strlen(inputText)] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(inputText);
            if (len > 0) inputText[len - 1] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER) && strlen(inputText) > 0) {
            if (isEditMode && editIndex >= 0) {
                strcpy(carousel.options[editIndex].text, inputText);
                isEditMode = false;
                editIndex = -1;
            } else {
                AddOption(&carousel, inputText);
            }
            SaveOptions(&carousel);
            memset(inputText, 0, MAX_OPTION_LENGTH);
        }

        UpdateCarouselSpin(&carousel);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawCarousel(&carousel);

        DrawRectangle(10, 550, 780, 40, LIGHTGRAY);
        DrawText(inputText, 20, 560, 20, BLACK);
        DrawText(isEditMode ? "Edit Mode" : "Add Option", 20, 520, 20, BLACK);

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
                strcpy(inputText, carousel.options[i].text);
                isEditMode = true;
                editIndex = i;
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
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

void SaveOptions(Carousel *carousel) {
#ifdef __EMSCRIPTEN__
    // Convert carousel data to string format
    char buffer[4096] = {0};
    char *ptr = buffer;
    
    // Write count
    ptr += sprintf(ptr, "%d;", carousel->count);
    
    // Write each option
    for (int i = 0; i < carousel->count; i++) {
        ptr += sprintf(ptr, "%s,%d,%d,%d;", 
            carousel->options[i].text,
            carousel->options[i].color.r,
            carousel->options[i].color.g,
            carousel->options[i].color.b
        );
    }
    
    // Save to localStorage
    EM_ASM({
        localStorage.setItem('carouselData', UTF8ToString($0));
    }, buffer);
#else
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
    // Get data from localStorage
    char* data = (char*)EM_ASM_INT({
        var data = localStorage.getItem('carouselData');
        if (!data) return 0;
        
        var lengthBytes = lengthBytesUTF8(data) + 1;
        var stringOnWasmHeap = _malloc(lengthBytes);
        stringToUTF8(data, stringOnWasmHeap, lengthBytes);
        return stringOnWasmHeap;
    });
    
    if (!data) {
        carousel->count = 0;
        return;
    }
    
    // Parse the data
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
                carousel->options[i].color = (Color){
                    (unsigned char)atoi(r),
                    (unsigned char)atoi(g),
                    (unsigned char)atoi(b),
                    255
                };
                i++;
            }
        }
        carousel->count = i;  // Update count to actual number parsed
    }
    
    // Free the allocated memory
    free(data);
#else
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