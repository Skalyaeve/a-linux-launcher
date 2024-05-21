#include "../include/header.h"

volatile sig_atomic_t stop = False;
void sig_hdl(const int sig){ (void)sig; stop = True; }

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
        value = strtok(NULL, "=");
        value[strlen(value) - 1] = '\0';

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

    if (!cfg->terminal || !cfg->shell
        || !cfg->browser || !cfg->search_engine){
        fprintf(stderr, "Missing required key\n");
        return FAILURE;
    }
    return read == -1 ? SUCCESS : errno;
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
    if (setwindows(menu->display, menu->root) == FAILURE)
        return errno;
    return SUCCESS;
}

byte setwindows(Display* const display, Windows* const ptr){
    if (setprops(display, &ptr->window) == FAILURE) return FAILURE;
    XSelectInput(display, ptr->window, ExposureMask | KeyPressMask);
    Entry* entry = ptr->entries;
    while (entry){
        if (entry->child)
            if (setwindows(display, entry->child) == FAILURE)
                return FAILURE;
        entry = entry->next;
    }
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

    if (menu->root) free_window(menu->root);
    if (menu->search) free_window(menu->search);
    if (menu->gc) XFreeGC(menu->display, menu->gc);
    if (menu->display) XCloseDisplay(menu->display);
    unlink(LOCKFILE);
    return code;
}
