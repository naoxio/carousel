#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h" 
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


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

int main(void) {
    InitWindow(800, 600, "Carousel Options");
    SetTargetFPS(60);

    Carousel carousel = {0};
    carousel.currentAngle = 0.0f;
    carousel.isSpinning = false;
    
    LoadOptions(&carousel);

    char inputText[MAX_OPTION_LENGTH] = {0};
    bool isEditMode = false;
    int editIndex = -1;

    while (!WindowShouldClose()) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointCircle(mousePos, (Vector2){400, 300}, 150)) {
                if (!carousel.isSpinning && carousel.count > 0) {
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

    // Draw sectors
    for (int i = 0; i < carousel->count; i++) {
        float startAngle = carousel->currentAngle + i * sectorAngle;
        // Replace DrawSectorPro with DrawCircleSector
        DrawCircleSector((Vector2){centerX, centerY}, radius, 
                        startAngle, startAngle + sectorAngle, 
                        32, carousel->options[i].color);
        
        // Draw text
        float textAngle = startAngle + sectorAngle / 2;
        float textX = centerX + cosf(textAngle * DEG2RAD) * (radius * 0.7f);
        float textY = centerY + sinf(textAngle * DEG2RAD) * (radius * 0.7f);
        
        Vector2 textPos = {textX, textY};
        float rotation = textAngle + 90;
        DrawTextPro(GetFontDefault(), carousel->options[i].text,
                   textPos, (Vector2){0, 0}, rotation, 20, 2, WHITE);
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
    FILE *file = fopen("carousel_options.dat", "wb");
    if (file) {
        fwrite(&carousel->count, sizeof(int), 1, file);
        fwrite(carousel->options, sizeof(Option), carousel->count, file);
        fclose(file);
    }
}

void LoadOptions(Carousel *carousel) {
    FILE *file = fopen("carousel_options.dat", "rb");
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
        carousel->currentAngle += carousel->spinSpeed;
        carousel->spinSpeed *= 0.98f; // Deceleration

        if (carousel->spinSpeed < 0.1f) {
            carousel->isSpinning = false;
            AdjustToNearestSector(carousel);
        }

        // Keep angle within 360 degrees
        while (carousel->currentAngle >= 360.0f) {
            carousel->currentAngle -= 360.0f;
        }
    }
}

void AdjustToNearestSector(Carousel *carousel) {
    float sectorAngle = GetSectorAngle(carousel->count);
    float targetSector = roundf(carousel->currentAngle / sectorAngle);
    carousel->currentAngle = targetSector * sectorAngle;
}
