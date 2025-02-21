#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "carousel.h"
#include "input_handler.h"
#include "theme_manager.h"
#include <string.h>

char globalInputText[MAX_OPTION_LENGTH] = {0};
bool globalIsEditMode = false;
int globalEditIndex = -1;

Carousel* globalCarousel = NULL;
static float cursorBlinkTime = 0.0f;
static bool showCursor = true;
int screenWidth = 800;
int screenHeight = 600;
Rectangle submitButton;

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Carousel");
    SetTargetFPS(60);

    InitThemeManager();

    Carousel carousel = {0};
    carousel.currentAngle = -90.0f;
    carousel.isSpinning = false;
    
    LoadOptions(&carousel);
    globalCarousel = &carousel;

    while (!WindowShouldClose()) {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        UpdateThemeManager(); 

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            float centerX = screenWidth / 2;
            float centerY = screenHeight / 2;
            float radius = fmin(screenWidth, screenHeight) * 0.25f;
            
            if (CheckCollisionPointCircle(mousePos, (Vector2){centerX, centerY}, radius)) {
                if (carousel.count > 0) {
                    carousel.isSpinning = true;
                    carousel.spinSpeed = 20.0f + (float)GetRandomValue(0, 10);
                    carousel.targetAngle = carousel.currentAngle + 
                        (float)GetRandomValue(720, 1440);
                }
            }
        }

        #ifndef __EMSCRIPTEN__
        int key = GetCharPressed();
        while (key > 0) {
            ProcessKeyPress(key);
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            ProcessKeyPress(KEY_BACKSPACE);
        }

        if (IsKeyPressed(KEY_ENTER)) {
            ProcessKeyPress(KEY_ENTER);
        }
        #endif

        UpdateCarouselSpin(&carousel);

        BeginDrawing();
        Theme currentTheme = GetCurrentTheme();
        ClearBackground(currentTheme.background);

        DrawThemeSelector();

        cursorBlinkTime += GetFrameTime();
        if (cursorBlinkTime >= 0.5f) {
            cursorBlinkTime = 0;
            showCursor = !showCursor;
        }
        DrawCarousel(&carousel, screenWidth, screenHeight);
        
        float inputWidth = screenWidth - 120;
        float inputY = screenHeight - 50;
        float labelY = screenHeight - 80;

        float submitX = screenWidth - 100;
        Rectangle submitButton = (Rectangle){submitX, inputY, 80, 40};

        // Update UI elements with theme colors
        DrawRectangle(10, inputY, inputWidth, 40, currentTheme.secondary);
        DrawText(globalInputText, 20, inputY + 10, 20, currentTheme.text);
        DrawText(globalIsEditMode ? "Edit Mode" : "Add Option", 20, labelY, 20, currentTheme.text);

        if (showCursor) {
            int textWidth = MeasureText(globalInputText, 20);
            DrawRectangle(20 + textWidth, inputY + 10, 2, 20, currentTheme.text);
        }

        // Use theme colors for the list items
        float maxListHeight = screenHeight - 100;
        float itemHeight = 30;

        for (int i = 0; i < carousel.count; i++) {
            float yPos = 50 + (i * itemHeight);
            
            if (yPos >= 50 && yPos < maxListHeight) {
                DrawText(TextFormat("%d: %s", i + 1, carousel.options[i].text), 
                        20, yPos, 20, currentTheme.text);
                
                Rectangle delButton = {220, yPos, 60, 20};
                // You can customize GuiSetStyle here for buttons if needed
                if (GuiButton(delButton, "Delete")) {
                    DeleteOption(&carousel, i);
                    SaveOptions(&carousel);
                }

                Rectangle editButton = {290, yPos, 60, 20};
                if (GuiButton(editButton, "Edit")) {
                    strcpy(globalInputText, carousel.options[i].text);
                    globalIsEditMode = true;
                    globalEditIndex = i;
                }
            }
        }

        // Add submit button
        if (GuiButton(submitButton, "Submit")) {
            if (strlen(globalInputText) > 0) {
                if (globalIsEditMode && globalEditIndex >= 0) {
                    strcpy(globalCarousel->options[globalEditIndex].text, globalInputText);
                    globalIsEditMode = false;
                    globalEditIndex = -1;
                } else {
                    AddOption(globalCarousel, globalInputText);
                }
                SaveOptions(globalCarousel);
                memset(globalInputText, 0, MAX_OPTION_LENGTH);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}