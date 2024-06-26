#include "../include/header.h"

char* get_realpath(const char* const path){
    if (path[0] == '~'){
        const char* const home = getenv("HOME");
        if (!home) return NULL;

        const size_t len = strlen(home) + strlen(path);
        char* const new_path = malloc(len);
        if (!new_path) return NULL;
        snprintf(new_path, len, "%s%s", home, path + 1);
        return new_path;
    }
    return (path[0] != '/') ? realpath(path, NULL) : strdup(path);
}

void free_strlist(Strlist* list){
    Strlist* ptr = list;
    while (ptr){
        ptr = list->next;
        free(list->str);
        free(list);
        list = ptr;
    }
}

void free_chardlist(Chardlist* list){
    Chardlist* ptr = list;
    while (ptr){
        ptr = list->next;
        free(list);
        list = ptr;
    }
}

void ft_strip(char** str){
    size_t len = strlen(*str);
    while (len && isspace((*str)[len - 1])) (*str)[len-- - 1] = '\0';
    while (isspace(**str)) (*str)++;
}
