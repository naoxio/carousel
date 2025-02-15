#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdbool.h>
#include "raylib.h"
#include "carousel.h"

#define MAX_OPTION_LENGTH 50

extern char globalInputText[MAX_OPTION_LENGTH];
extern bool globalIsEditMode;
extern int globalEditIndex;
extern Carousel* globalCarousel;

char* GetGlobalInputText(void);
int GetInputTextLength(void);
void ProcessKeyPress(int key);

#endif