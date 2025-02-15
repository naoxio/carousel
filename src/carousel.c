#include "carousel.h"
#include "raylib.h"
#include "ui_manager.h"
#include <math.h>
#include "theme_manager.h"

void DrawCarousel(Carousel *carousel) {
    if (carousel->count == 0) return;

    Theme currentTheme = GetCurrentTheme();
    float centerX = screenWidth / 2;
    float centerY = screenHeight / 2;
    float radius = fmin(screenWidth, screenHeight) * 0.25f;
    float sectorAngle = GetSectorAngle(carousel->count);

    for (int i = 0; i < carousel->count; i++) {
        float startAngle = carousel->currentAngle + i * sectorAngle - 90;
        
        // Blend the original color with the theme's primary color
        Color sectorColor = carousel->options[i].color;
        Color blendedColor = (Color){
            (sectorColor.r + currentTheme.primary.r) / 2,
            (sectorColor.g + currentTheme.primary.g) / 2,
            (sectorColor.b + currentTheme.primary.b) / 2,
            255
        };
        
        DrawCircleSector((Vector2){centerX, centerY}, radius, 
                        startAngle, startAngle + sectorAngle, 
                        32, blendedColor);
        
        float textAngle = startAngle + sectorAngle / 2;
        float textX = centerX + cosf(textAngle * DEG2RAD) * (radius * 0.6f);
        float textY = centerY + sinf(textAngle * DEG2RAD) * (radius * 0.6f);
        
        Vector2 textPos = {textX, textY};
        int textWidth = MeasureText(carousel->options[i].text, 20);
        Vector2 textOrigin = {textWidth/2.0f, 10.0f};
        
        // Use theme text color
        DrawTextPro(GetFontDefault(), carousel->options[i].text,
                   textPos, textOrigin, textAngle, 20, 1, currentTheme.text);
    }

    // Use theme accent color for the pointer
    DrawTriangle(
        (Vector2){centerX, centerY - radius - 20},
        (Vector2){centerX - 10, centerY - radius},
        (Vector2){centerX + 10, centerY - radius},
        currentTheme.accent
    );
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

void UpdateCarouselSpin(Carousel *carousel) {
    if (carousel->isSpinning) {
        if (carousel->spinSpeed > 1.0f) {
            carousel->currentAngle += carousel->spinSpeed;
            carousel->spinSpeed *= 0.97f;
        } else {
            float sectorAngle = GetSectorAngle(carousel->count);
            float halfSectorAngle = sectorAngle / 2.0f;
            
            float normalizedAngle = carousel->currentAngle + halfSectorAngle;
            int sector = floor(normalizedAngle / sectorAngle);
            float targetAngle = (sector * sectorAngle) - halfSectorAngle;
            
            float angleDiff = targetAngle - carousel->currentAngle;
            if (angleDiff > 180.0f) angleDiff -= 360.0f;
            if (angleDiff < -180.0f) angleDiff += 360.0f;
            
            float moveSpeed = angleDiff * 0.1f;
            carousel->currentAngle += moveSpeed;
            
            if (fabs(moveSpeed) < 0.01f) {
                carousel->isSpinning = false;
                carousel->currentAngle = targetAngle;
                carousel->spinSpeed = 0.0f;
            }
        }

        while (carousel->currentAngle >= 360.0f) carousel->currentAngle -= 360.0f;
        while (carousel->currentAngle < 0.0f) carousel->currentAngle += 360.0f;
    }
}
