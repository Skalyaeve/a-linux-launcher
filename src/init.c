#include "../include/header.h"

byte init(Menu* const menu, Config* const cfg){
    Stat s;
    if (stat(cfg->path, &s) == -1){
        perror("stat");
        return errno;
    }
    if (!S_ISDIR(s.st_mode)){
        fprintf(stderr, "%s is not a directory\n", path);
        return FAILURE;
    }
    menu->root = new_window(cfg->path, menu, cfg);
    if (!menu->root) return errno;

    menu->gc = XCreateGC(menu->display, menu->root->window, 0, NULL);
    if (!menu->gc){
        perror("XCreateGC");
        return errno;
    }
    return SUCCESS;
}

Windows* new_window(const char* const path, Menu* const menu,
                    Config* const cfg){
    Windows* window = malloc(sizeof(Windows));
    if (!window){
        perror("malloc");
        return NULL;
    }
    memset(window, 0, sizeof(Windows));
    Entry** ptr = &window->entries;

    DIR* dir = opendir(path);
    if (!dir){
        free(window);
        perror("opendir");
        return NULL;
    }
    Dirent* entry;
    while ((entry = readdir(dir))){
        if (entry->d_type == DT_DIR){
        }
    }
}
