#ifndef STORAGE_H
#define STORAGE_H


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE void js_saveToLocalStorage(const char* key, const char* value);
EMSCRIPTEN_KEEPALIVE char* js_loadFromLocalStorage(const char* key);
#else
char* get_data_path(void);
#endif

#endif
