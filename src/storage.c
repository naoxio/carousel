#include "storage.h"
#include "carousel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __EMSCRIPTEN__
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>
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
    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/options.dat", get_data_path());
    FILE *file = fopen(filepath, "rb");
    if (file) {
        size_t read;
        read = fread(&carousel->count, sizeof(int), 1, file);
        if (read != 1) carousel->count = 0;
        read = fread(carousel->options, sizeof(Option), carousel->count, file);
        if (read != carousel->count) carousel->count = 0;
        fclose(file);
    } else {
        carousel->count = 0;
    }
#endif
}

#ifndef __EMSCRIPTEN__
char* get_data_path(void) {
    static char path[1024];
    const char *home;
    
    home = getenv("XDG_DATA_HOME");
    if (home != NULL) {
        snprintf(path, sizeof(path), "%s/carousel", home);
    } else {
        home = getenv("HOME");
        if (home == NULL) {
            home = getpwuid(getuid())->pw_dir;
        }
        snprintf(path, sizeof(path), "%s/.local/share/carousel", home);
    }
    
    mkdir(path, 0755);
    
    return path;
}
#endif
