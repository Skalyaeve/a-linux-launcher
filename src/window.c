#include "../include/header.h"

extern volatile sig_atomic_t stop;

Windows* create_window(const char* const path, Menu* const menu,
                       Config* const cfg, const size_t x_offset,
                       const size_t y_offset){
    if (stop) return NULL;
    Windows* window = malloc(sizeof(Windows));
    if (!window){
        perror("malloc");
        return NULL;
    }
    memset(window, 0, sizeof(Windows));
    Entry** ptr = &window->entries;

    Intlist* const childs = malloc(sizeof(Intlist));
    if (!childs){
        free(window);
        perror("malloc");
        return NULL;
    }
    memset(childs, 0, sizeof(Intlist));

    DIR* dir = opendir(path);
    if (!dir){
        free(window);
        perror("opendir");
        return NULL;
    }
    size_t count = 0;
    size_t len = 0;
    size_t largest = 0;
    Dirent* entry;
    while ((entry = readdir(dir))){
        *ptr = malloc(sizeof(Entry));
        if (!*ptr){
            perror("malloc");
            break;
        }
        memset(*ptr, 0, sizeof(Entry));
        if (entry->d_type == DT_DIR){

            (*ptr)->name = strdup(entry->d_name);
            if (!(*ptr)->name){
                perror("strdup");
                break;
            }
            //(*ptr)->window = create_window(entry->d_name, menu, cfg);
            //if (!(*ptr)->window) break;
        }
        else if (new_app(entry->d_name, *ptr, cfg) != SUCCESS) break;

        len = strlen((*ptr)->name);
        if (len > largest) largest = len;
        count++;
        ptr = &(*ptr)->next;
    }
    closedir(dir);
    if (entry || !count)
        free_intlist(childs);
        free_window(window);
        return NULL;
    }
    window->window = new_window(menu, cfg, largest,
                                count, x_offset, y_offset);
    if (!window->window){
        free_window(window);
        return NULL;
    }
    return window;
}

void free_window(Windows* window){
    Entry* entry = window->entries;
    Entry* next;
    while (entry){
        next = entry->next;
        if (entry->name) free(entry->name);
        if (entry->window) free_window(entry->window);
        entry = next;
    }
    free(window);
}

int new_app(const char* const path, Entry* const entry,
            Config* const cfg){
    FILE* file = fopen(path, "r");
    if (!file){
        perror("fopen");
        return errno;
    }
    Exec exec = {0};
    ssize_t read;
    char* line = NULL;
    size_t len = 0;
    char* key;
    char* value;
    while ((read = getline(&line, &len, file)) != -1){
        key = strtok(line, "=");
        value = strtok(NULL, "=");

        if (!strcmp(key, "Type")){
            if (!strcmp(value, "Application\n"))
                exec.type = APPLICATION;
            else if (!strcmp(value, "Link\n"))
                exec.type = LINK;
            else fprintf(stderr,
                         "Invalid value for Type: %s\n", value);
        }
        else if (!strcmp(key, "Name")){
            entry->name = strdup(value);
            if (!entry->name){
                perror("strdup");
                break;
            }
        }
        else if (!strcmp(key, "Exec") || !strcmp(key, "URL")){
            exec.cmd = strdup(value);
            if (!exec.cmd){
                perror("strdup");
                break;
            }
        }
        else if (!strcmp(key, "Terminal")){
            if (!strcmp(value, "true\n")) exec.terminal = YES;
            else if (!strcmp(value, "false\n")) exec.terminal = NO;
            else fprintf(stderr,
                        "Invalid value for Terminal: %s\n", value);
        }
    }
    fclose(file);
    if (line) free(line);
    if (read != -1){
        if (exec.cmd) free(exec.cmd);
        return errno;
    }
    if (!entry->name || !exec.cmd || !exec.type){
        fprintf(stderr, "Missing required field(s) in %s\n", path);
        if (exec.cmd) free(exec.cmd);
        return EINVAL;
    }
    entry->exec = fill_exec(exec, cfg);
    free(exec.cmd);
    if (!entry->exec) return errno;
    return SUCCESS;
}

char* fill_exec(Exec exec, Config* const cfg){
    char* cmd = NULL;;
    char* shell = strdup(cfg->shell);
    if (!shell){
        perror("strdup");
        return NULL;
    }
    char* ptr;
    size_t len;
    switch (exec.type){
        case APPLICATION:
            len = strlen(cfg->terminal) + strlen(shell)
                + strlen(exec.cmd) + 5;
            if (exec.terminal){
                ptr = strtok(shell, " ")
                len += 7 + strlen(ptr);
            }
            cmd = malloc(len);
            if (!cmd){
                perror("malloc");
                break;
            }
            if (exec.terminal)
                snprintf(cmd, len, "%s %s '%s; exec %s'",
                         cfg->terminal, shell, exec.cmd, ptr);
            else snprintf(cmd, len, "%s %s '%s'",
                          cfg->terminal, shell, exec.cmd);
        case LINK:
            len = strlen(cfg->browser) + strlen(exec.cmd) + 4;
            cmd = malloc(len);
            if (!cmd){
                perror("malloc");
                break;
            }
            snprintf(cmd, len, "%s '%s'", cfg->browser, exec.cmd);
    }
    free(shell);
    return cmd;
}
