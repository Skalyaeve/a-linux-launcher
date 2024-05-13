#include "../include/header.h"

char* get_realpath(const char* const path){
    if (path[0] == '~'){
        const char* const home = getenv("HOME");
        if (!home) return NULL;

        size_t len = strlen(home) + strlen(path);
        char* const new_path = malloc(len);
        if (!new_path) return NULL;
        snprintf(new_path, len, "%s%s", home, path + 1);
        return new_path;
    }
    else if (path[0] != '/') return realpath(path, NULL);
    return strdup(path);
}
