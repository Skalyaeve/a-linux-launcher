#include "../include/header.h"

extern volatile sig_atomic_t stop;

Windows* create_window(const char* const path,
                       Menu* const menu, Config* const cfg,
                       size_t x_offset, size_t y_offset){
    if (stop) return NULL;
    Windows* window = malloc(sizeof(Windows));
    if (!window){
        perror("malloc");
        return NULL;
    }
    memset(window, 0, sizeof(Windows));
    Entry** entries = &window->entries;

    Intlist* childs = NULL;
    Intlist** listptr = &childs;

    DIR* const dir = opendir(path);
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
        *entries = malloc(sizeof(Entry));
        if (!*entries){
            perror("malloc");
            break;
        }
        memset(*entries, 0, sizeof(Entry));
        if (entry->d_type == DT_DIR){

            (*entries)->name = strdup(entry->d_name);
            if (!(*entries)->name){
                perror("strdup");
                break;
            }
            *listptr = malloc(sizeof(Intlist));
            if (!*listptr){
                perror("malloc");
                break;
            }
            (*listptr)->value = count;
            (*listptr)->next = NULL;
            listptr = &(*listptr)->next;
        }
        else if (new_app(entry->d_name, *entries, cfg) != SUCCESS)
            break;

        len = strlen((*entries)->name);
        if (len > largest) largest = len;
        count++;
        entries = &(*entries)->next;
    }
    closedir(dir);
    if (entry || !count){
        free_window(window);
        if (childs) free_intlist(childs);
        return NULL;
    }
    if (childs){
        size_t index;
        listptr = &childs;
        x_offset += largest + cfg->x_padding + cfg->border_size;
        while (*listptr){
            index = (*listptr)->value;
            y_offset = cfg->font_size * index
                + cfg->spacing * index + cfg->y_padding
                + cfg->border_size;

            entries[index]->child = create_window(
                entries[index]->name, menu, cfg, x_offset, y_offset);
            if (!entries[index]->child) break;
            listptr = &(*listptr)->next;
        }
        free_intlist(childs);
        if (*listptr){
            free_window(window);
            return NULL;
        }
    }
    window->window = XCreateSimpleWindow(
        menu->display, RootWindow(menu->display, menu->screen),
        cfg->x + x_offset, cfg->y + y_offset,
        largest + cfg->x_padding * 2, cfg->font_size * count
        + cfg->spacing * (count - 1) + cfg->x_padding * 2,
        cfg->border_size, cfg->border_color.pixel,
        cfg->bg_color.pixel);
    if (!window->window){
        free_window(window);
        window = NULL;
    }
    return window;
}

void free_window(Windows* const window){
    Entry* entry = window->entries;
    Entry* next;
    while (entry){
        next = entry->next;
        if (entry->name) free(entry->name);
        if (entry->exec) free(entry->exec);
        if (entry->child) free_window(entry->child);
        free(entry);
        entry = next;
    }
    free(window);
}

int new_app(const char* const path, Entry* const entry,
            Config* const cfg){
    FILE* const file = fopen(path, "r");
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
                exec.type = APP;
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
    entry->exec = fill_exec(&exec, cfg);
    free(exec.cmd);
    if (!entry->exec) return errno;
    return SUCCESS;
}

char* fill_exec(Exec* exec, Config* const cfg){
    char* cmd = NULL;;
    char* shell = strdup(cfg->shell);
    if (!shell){
        perror("strdup");
        return NULL;
    }
    char* ptr;
    size_t len;
    switch (exec->type){
        case APP:
            len = strlen(exec->cmd) + 1;
            if (exec->terminal){
                ptr = strtok(shell, " ");
                len += strlen(cfg->terminal) + strlen(shell)
                    + strlen(ptr) + 11;
            }
            cmd = malloc(len);
            if (!cmd){
                perror("malloc");
                break;
            }
            if (exec->terminal)
                snprintf(cmd, len, "%s %s '%s; exec %s'",
                         cfg->terminal, shell, exec->cmd, ptr);
            else snprintf(cmd, len, "%s", exec->cmd);
            break;
        case LINK:
            len = strlen(cfg->browser) + strlen(exec->cmd) + 4;
            cmd = malloc(len);
            if (!cmd){
                perror("malloc");
                break;
            }
            snprintf(cmd, len, "%s '%s'", cfg->browser, exec->cmd);
            break;
    }
    free(shell);
    return cmd;
}
