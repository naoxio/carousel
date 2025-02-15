#ifndef STORAGE_H
#define STORAGE_H

#ifdef __EMSCRIPTEN__
void js_saveToLocalStorage(const char* key, const char* value);
char* js_loadFromLocalStorage(const char* key);
#else
char* get_data_path(void);
#endif

#endif
