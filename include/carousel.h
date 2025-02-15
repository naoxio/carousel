#ifndef CAROUSEL_H
#define CAROUSEL_H

#include "raylib.h"
#include <stdbool.h>
#include <string.h>

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

void SaveOptions(Carousel *carousel);
void LoadOptions(Carousel *carousel);
void AddOption(Carousel *carousel, const char *text);
void DeleteOption(Carousel *carousel, int index);
void DrawCarousel(Carousel *carousel);
Color GenerateRandomColor(void);
void UpdateCarouselSpin(Carousel *carousel);
float GetSectorAngle(int optionCount);
void AdjustToNearestSector(Carousel *carousel);

#endif
