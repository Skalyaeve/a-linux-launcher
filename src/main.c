#include "../include/header.h"

volatile sig_atomic_t stop = False;
void sig_hdl(const int sig){
    if (sig == SIGSEGV){
        unlink(LOCKFILE);
        fprintf(stderr, "That's why you need to sleep\n");
        exit(FAILURE);
    }
    stop = True;
}

int main(int ac, char** av){
    FILE* lock = fopen(LOCKFILE, "r");
    if (lock){
        int pid;
        fscanf(lock, "%d", &pid);
        fclose(lock);
        kill(pid, SIGTERM);
        return SUCCESS;
    }
    lock = fopen(LOCKFILE, "w");
    if (!lock){
        perror("fopen");
        return errno;
    }
    fprintf(lock, "%d", getpid());
    fclose(lock);
    if (ac < 2){
        fprintf(stderr, "Usage: %s <config>\n", av[0]);
        return FAILURE;
    }
    Sigaction sa;
    sa.sa_handler = sig_hdl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGSEGV, &sa, NULL);

    Config cfg = {0};
    Menu menu = {0};

    menu.display = XOpenDisplay(NULL);
    if (!menu.display){
        perror("XOpenDisplay");
        return errno;
    }
    menu.screen = DefaultScreen(menu.display);
    byte code;

    code = config(av[1], &cfg, menu.display, menu.screen);
    if (code != SUCCESS) return bye(code, &cfg, &menu);
    if (stop) return bye(SUCCESS, &cfg, &menu);

    code = init(&menu, &cfg);
    if (code != SUCCESS) return bye(code, &cfg, &menu);
    if (stop) return bye(SUCCESS, &cfg, &menu);

    menu.root->visible = YES;
    menu.focus = menu.root;
    menu.active = menu.root;
    XMoveWindow(menu.display, menu.root->window, cfg.x, cfg.y);
    XMapWindow(menu.display, menu.root->window);
    return bye(update(&menu, &cfg), &cfg, &menu);
}

int config(const char* const path, Config* const cfg,
            Display* const display, const int screen){
    Colormap colormap = DefaultColormap(display, screen);
    FILE* const file = fopen(path, "r");
    if (!file){
        perror("fopen");
        return errno;
    }
    ssize_t read;
    char* line = NULL;
    size_t len = 0;
    char* key;
    char* value;
    while ((read = getline(&line, &len, file)) != -1){
        if (line[0] == '#' || line[0] == '\n') continue;
        key = strtok(line, "=");
        value = line + strlen(key) + 1;
        ft_strip(&key);
        ft_strip(&value);

        if (!strcmp(key, "path")){
            cfg->path = get_realpath(value);
            if (!cfg->path){
                perror("get_realpath");
                break;
            }
        }
        else if (!strcmp(key, "x")) cfg->x = atoi(value);
        else if (!strcmp(key, "y")) cfg->y = atoi(value);

        else if (!strcmp(key, "border-size"))
            cfg->border_size = atoi(value);

        else if (!strcmp(key, "border-color")){
            XParseColor(display, colormap, value,
                        &cfg->border_color);
            XAllocColor(display, colormap, &cfg->border_color);
        }
        else if (!strcmp(key, "bg-color")){
            XParseColor(display, colormap, value, &cfg->bg_color);
            XAllocColor(display, colormap, &cfg->bg_color);
        }
        else if (!strcmp(key, "fg-color")){
            XParseColor(display, colormap, value, &cfg->fg_color);
            XAllocColor(display, colormap, &cfg->fg_color);
        }
        else if (!strcmp(key, "focus-bg-color")){
            XParseColor(display, colormap, value,
                        &cfg->focus_bg_color);
            XAllocColor(display, colormap, &cfg->focus_bg_color);
        }
        else if (!strcmp(key, "focus-fg-color")){
            XParseColor(display, colormap, value,
                        &cfg->focus_fg_color);
            XAllocColor(display, colormap, &cfg->focus_fg_color);
        }
        else if (!strcmp(key, "x-padding"))
            cfg->x_padding = atoi(value);

        else if (!strcmp(key, "y-padding"))
            cfg->y_padding = atoi(value);

        else if (!strcmp(key, "font-size"))
            cfg->font_size = atoi(value);

        else if (!strcmp(key, "line-margin"))
            cfg->line_margin = atoi(value);

        else if (!strcmp(key, "window-margin"))
            cfg->window_margin = atoi(value);

        else if (!strcmp(key, "max-len"))
            cfg->max_len = atoi(value);

        else if (!strcmp(key, "max-lines"))
            cfg->max_lines= atoi(value);

        else if (!strcmp(key, "font")){
            cfg->font = strdup(value);
            if (!cfg->font){
                perror("strdup");
                break;
            }
        }
        else if (!strcmp(key, "terminal")){
            cfg->terminal = strdup(value);
            if (!cfg->terminal){
                perror("strdup");
                break;
            }
        }
        else if (!strcmp(key, "shell")){
            cfg->shell = strdup(value);
            if (!cfg->shell){
                perror("strdup");
                break;
            }
        }
        else if (!strcmp(key, "browser")){
            cfg->browser = strdup(value);
            if (!cfg->browser){
                perror("strdup");
                break;
            }
        }
        else if (!strcmp(key, "search-engine")){
            cfg->search_engine = strdup(value);
            if (!cfg->search_engine){
                perror("strdup");
                break;
            }
        }
        else fprintf(stderr, "Unknown key: %s\n", key);
    }
    fclose(file);
    if (line) free(line);
    return read == -1 ? check_args(cfg) : errno;
}

int check_args(Config* const cfg){
    if (!cfg->path){
        fprintf(stderr, "No path specified\n");
        return FAILURE;
    }
    if (!cfg->font){
        fprintf(stderr, "No font specified\n");
        return FAILURE;
    }
    if (!cfg->terminal){
        fprintf(stderr, "No terminal specified\n");
        return FAILURE;
    }
    if (!cfg->shell){
        fprintf(stderr, "No shell specified\n");
        return FAILURE;
    }
    if (!cfg->browser){
        fprintf(stderr, "No browser specified\n");
        return FAILURE;
    }
    if (!cfg->search_engine){
        fprintf(stderr, "No search engine specified\n");
        return FAILURE;
    }
    if (!cfg->font_size){
        fprintf(stderr, "Font size should be greater than 0\n");
        return FAILURE;
    }
    if (!cfg->max_len){
        fprintf(stderr, "Max length should be greater than 0\n");
        return FAILURE;
    }
    if (!cfg->max_lines){
        fprintf(stderr, "Max lines should be greater than 0\n");
        return FAILURE;
    }
    return SUCCESS;
}

int init(Menu* const menu, Config* const cfg){
    Stat s;
    if (stat(cfg->path, &s) == -1){
        strerror(errno);
        return errno;
    }
    if (!S_ISDIR(s.st_mode)){
        fprintf(stderr, "%s is not a directory\n", cfg->path);
        return FAILURE;
    }
    menu->root = create_window(cfg->path, menu, cfg, 0, 0);
    if (!menu->root) return FAILURE;

    menu->gc = XCreateGC(menu->display, menu->root->window, 0, NULL);
    if (!menu->gc){
        perror("XCreateGC");
        return errno;
    }
    XFontStruct* const font = XLoadQueryFont(menu->display,
                                             cfg->font);
    if (!font){
        perror("XLoadQueryFont");
        return errno;
    }
    XSetFont(menu->display, menu->gc, font->fid);
    XFreeFont(menu->display, font);
    if (setwindows(menu->display, menu->root,
                   &menu->root->window) == FAILURE)
        return errno;
    return SUCCESS;
}

byte setwindows(Display* const display, Windows* const ptr,
                Window* const window){
    if (setprops(display, window) == FAILURE) return FAILURE;
    XSelectInput(display, *window, ExposureMask | KeyPressMask);
    ptr->draw_start = ptr->entries;
    ushort count = ptr->count;
    Entry* entry = ptr->entries;
    while (entry){
        if (entry->child
            && setwindows(display, entry->child,
                          &entry->child->window) == FAILURE)
                return FAILURE;
        if (count && !--count) ptr->draw_end = entry;
        entry = entry->next;
    }
    if (!ptr->draw_end) ptr->draw_end = entry;
    return SUCCESS;
}

byte setprops(Display* const display, Window* const window){
    Atom window_type = XInternAtom(
        display, "_NET_WM_WINDOW_TYPE", False);
    if (!window_type){
        perror("XInternAtom");
        return FAILURE;
    }
    Atom window_type_dialog = XInternAtom(
        display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    if (!window_type_dialog){
        perror("XInternAtom");
        return FAILURE;
    }
    if (!XChangeProperty(
        display, *window, window_type, XA_ATOM, 32,
        PropModeReplace, (uchar *) &window_type_dialog, 1)){
        perror("XChangeProperty");
        return FAILURE;
    }
    Atom hints = XInternAtom( display, "_MOTIF_WM_HINTS", False);
    if (!hints){
        perror("XInternAtom");
        return FAILURE;
    }
    struct{
        const ulong flags;
        const ulong functions;
        const ulong decorations;
        const long input_mode;
        const ulong status;
    }s_hints = {2, 0, 0, 0, 0};

    if (!XChangeProperty(
        display, *window, hints, hints, 32,
        PropModeReplace, (uchar *)&s_hints, 5)){
        perror("XChangeProperty");
        return FAILURE;
    }
    return SUCCESS;
}

byte bye(const int code, Config* const cfg, Menu* const menu){
    if (cfg->path) free(cfg->path);
    if (cfg->font) free(cfg->font);
    if (cfg->terminal) free(cfg->terminal);
    if (cfg->shell) free(cfg->shell);
    if (cfg->browser) free(cfg->browser);
    if (cfg->search_engine) free(cfg->search_engine);

    if (menu->gc) XFreeGC(menu->display, menu->gc);
    if (menu->display) XCloseDisplay(menu->display);
    if (menu->root) free_window(menu->root);
    if (menu->search){
        if (menu->search->str) free_chardlist(menu->search->str);
        if (menu->search->result) free_window(menu->search->result);
        free(menu->search);
    }
    unlink(LOCKFILE);
    return code;
}
