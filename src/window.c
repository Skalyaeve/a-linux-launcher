#include "../include/header.h"

extern volatile sig_atomic_t stop;

Windows* create_window(const char* const path,
                       Menu* const menu, Config* const cfg,
                       ushort x_offset, ushort y_offset){
    if (stop) return NULL;
    Windows* window = malloc(sizeof(Windows));
    if (!window){
        perror("malloc");
        return NULL;
    }
    memset(window, 0, sizeof(Windows));
    Entry** entries = &window->entries;

    DIR* const dir = opendir(path);
    if (!dir){
        printf("Could not open %s\n", path);
        free(window);
        perror("opendir");
        return NULL;
    }
    ushort count = 0;
    ushort len = 0;
    ushort largest = 0;
    Dirent* entry;
    char* rpath = NULL;
    Strlist* order = NULL;
    Stat path_stat;
    while ((entry = readdir(dir))){
        if (!strcmp(entry->d_name, ".")
            || !strcmp(entry->d_name, ".."))
            continue;
        if (rpath) free(rpath);
        rpath = malloc(strlen(path) + strlen(entry->d_name) + 2);
        if (!rpath){
            perror("malloc");
            break;
        }
        snprintf(rpath, strlen(path) + strlen(entry->d_name) + 2,
                 "%s/%s", path, entry->d_name);
        *entries = malloc(sizeof(Entry));
        if (!*entries){
            perror("malloc");
            break;
        }
        memset(*entries, 0, sizeof(Entry));
        stat(rpath, &path_stat);
        if (S_ISDIR(path_stat.st_mode)){

            (*entries)->name = strdup(rpath);
            if (!(*entries)->name){
                perror("strdup");
                break;
            }
            (*entries)->have_child = YES;
            len = strlen(strstr((*entries)->name, path)
                         + strlen(path) + 1) + 4;
            if (len - 4 > cfg->max_len) len = cfg->max_len + 4;
        }
        else{
            if (!strcmp(entry->d_name, ".ord")){
                free(*entries);
                *entries = NULL;
                order = get_order(rpath);
                if (!order) break;
                continue;
            }
            if (new_app(rpath, *entries, cfg) != SUCCESS) break;
            len = strlen((*entries)->name);
        }
        if (len > largest) largest = len;
        count++;
        entries = &(*entries)->next;
    }
    if (rpath) free(rpath);
    closedir(dir);
    if (entry || !count){
        free_window(window);
        if (order) free_strlist(order);
        return NULL;
    }
    link_entries(window->entries);
    if (order){
        if (order_entries(window->entries, order, path)
            != SUCCESS){
            free_strlist(order);
            free_window(window);
            return NULL;
        }
        free_strlist(order);
    }
    if (create_childs(window, path, largest, x_offset, y_offset,
                      menu, cfg) != SUCCESS){
        free_window(window);
        return NULL;
    }
    window->x = cfg->x + x_offset;
    window->y = cfg->y + y_offset;
    window->largest = largest * cfg->font_size / 2
        + cfg->x_padding * 2;
    window->count = count;
    window->window = new_window(window, menu, cfg);
    if (!window->window){
        free_window(window);
        window = NULL;
    }
    return window;
}

void link_entries(Entry* entries){
    Entry* prev = NULL;
    while (entries){
        entries->prev = prev;
        prev = entries;
        entries = entries->next;
    }
}

bool create_childs(Windows* const window, const char* const path,
                   const ushort largest, ushort x_offset,
                   ushort y_offset, Menu* const menu,
                   Config* const cfg){
    char* ptr;
    char* substr;
    Entry* entries = window->entries;
    x_offset += largest * cfg->font_size / 2 + cfg->x_padding * 2
        + cfg->border_size * 2 + cfg->window_margin;
    size_t len;
    while (entries){
        if (!entries->have_child){
            entries = entries->next;
            continue;
        }
        entries->child = create_window(
            entries->name, menu, cfg, x_offset, y_offset);
        if (!entries->child) break;
        entries->child->parent = window;

        substr = strstr(entries->name, path) + strlen(path) + 1;
        ptr = malloc(largest + 1);
        if (!ptr){
            perror("strdup");
            break;
        }
        len = strlen(substr);
        if (len > cfg->max_len) len = cfg->max_len;
        memcpy(ptr, substr, len);
        memset(ptr + len, ' ', largest - len);
        ptr[largest - 1] = '>';
        ptr[largest] = '\0';
        entries->fullname = strdup(substr);
        free(entries->name);
        entries->name = ptr;
        y_offset += cfg->font_size + cfg->line_margin;
        entries = entries->next;
    }
    return SUCCESS;
}

void free_window(Windows* const window){
    free_entries(window->entries);
    free(window);
}

void free_entries(Entry* entries){
    Entry* next;
    while (entries){
        next = entries->next;
        if (entries->name) free(entries->name);
        if (entries->fullname) free(entries->fullname);
        if (entries->exec) free(entries->exec);
        if (entries->child) free_window(entries->child);
        free(entries);
        entries = next;
    }
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
        if (line[0] == '\n' || line[0] == '[') continue;
        if (!strstr(line, "=")){
            fprintf(stderr, "Invalid line in %s: %s", path, line);
            continue;
        }
        key = strtok(line, "=");
        value = strtok(NULL, "=");
        value[strlen(value) - 1] = '\0';

        if (!strcmp(key, "Type")){
            if (!strcmp(value, "Application")) exec.type = APP;
            else if (!strcmp(value, "Link")) exec.type = LINK;
            else fprintf(stderr,
                         "Invalid value for Type: %s", value);
        }
        else if (!strcmp(key, "Name")){
            size_t len = strlen(value);
            if (len > cfg->max_len) len = cfg->max_len;
            entry->name = strndup(value, len);
            if (!entry->name){
                perror("strdup");
                break;
            }
            entry->fullname = strdup(value);
            if (!entry->fullname){
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
            if (!strcmp(value, "true")) exec.terminal = YES;
            else if (!strcmp(value, "false")) exec.terminal = NO;
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

char* fill_exec(Exec* const exec, Config* const cfg){
    char* cmd = NULL;;
    char* shell = strdup(cfg->shell);
    if (!shell){
        perror("strdup");
        return NULL;
    }
    char* ptr;
    ushort len;
    switch (exec->type){
        case APP:
            len = strlen(exec->cmd) + 1;
            if (exec->terminal){
                ptr = strtok(shell, " ");
                len += strlen(cfg->terminal) + strlen(cfg->shell)
                    + strlen(ptr) + 11;
            }
            cmd = malloc(len);
            if (!cmd){
                perror("malloc");
                break;
            }
            if (exec->terminal)
                snprintf(cmd, len, "%s %s '%s; exec %s'",
                         cfg->terminal, cfg->shell, exec->cmd, ptr);
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

Strlist* get_order(const char* const path){
    FILE* const file = fopen(path, "r");
    if (!file){
        perror("fopen");
        return NULL;
    }
    ssize_t read;
    char* line = NULL;
    size_t len = 0;
    Strlist* order = NULL;
    Strlist** listptr = &order;
    while ((read = getline(&line, &len, file)) != -1){
        if (line[0] == '\n' || line[0] == '#') continue;
        *listptr = malloc(sizeof(Strlist));
        if (!*listptr){
            perror("malloc");
            break;
        }
        (*listptr)->str = strdup(line);
        if (!(*listptr)->str){
            perror("strdup");
            break;
        }
        (*listptr)->str[strlen((*listptr)->str) - 1] = '\0';
        (*listptr)->next = NULL;
        listptr = &(*listptr)->next;
    }
    fclose(file);
    if (line) free(line);
    if (read != -1) free_strlist(order);
    return read == -1 ? order : NULL;
}

byte order_entries(Entry* entries, Strlist* order,
                   const char* const path){
    Entry* ptr;
    Entry tmp = {0};
    char* str;
    while (order){
        ptr = entries;
        while (ptr){
            if (ptr->have_child)
                str = strdup(ptr->name + strlen(path) + 1);
            else str = strdup(ptr->fullname);
            if (!str){
                perror("strdup");
                return FAILURE;
            }
            if (!strcmp(str, order->str)){
                free(str);
                memcpy(&tmp, ptr, sizeof(Entry));
                ptr->name = entries->name;
                ptr->fullname = entries->fullname;
                ptr->exec = entries->exec;
                entries->name = tmp.name;
                entries->fullname = tmp.fullname;
                entries->exec = tmp.exec;
                entries = entries->next;
                break;
            }
            ptr = ptr->next;
            free(str);
        }
        order = order->next;
    }
    return SUCCESS;
}

Window new_window(Windows* const window, Menu* const menu,
                  Config* const cfg){
    if (window->count > cfg->max_lines)
        window->count = cfg->max_lines;
    return XCreateSimpleWindow(
        menu->display, RootWindow(menu->display, menu->screen),
        window->x, window->y, window->largest, cfg->font_size
        * window->count + cfg->line_margin * (window->count - 1)
        + cfg->y_padding * 2, cfg->border_size,
        cfg->border_color.pixel, cfg->bg_color.pixel);
}
