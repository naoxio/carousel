#include "input_handler.h"
#include "carousel.h"
#include <string.h>
#include <stdio.h>

char* GetGlobalInputText(void) {
    return globalInputText;
}

int GetInputTextLength(void) {
    return MAX_OPTION_LENGTH;
}

void ProcessKeyPress(int key) {
    if (key == KEY_BACKSPACE) {
        int len = strlen(globalInputText);
        if (len > 0) globalInputText[len - 1] = '\0';
    }
    else if (key == KEY_ENTER) {
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
    else if ((key >= 32) && (key <= 125)) {
        if (strlen(globalInputText) < MAX_OPTION_LENGTH - 1) {
            int len = strlen(globalInputText);
            globalInputText[len] = (char)key;
            globalInputText[len + 1] = '\0';
        }
    }
}
