#include "../include/header.h"

volatile sig_atomic_t stop = False;
void sig_hdl(int sig){ (void)sig; stop = True; }

int main(int ac, char** av){
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
    sigaction(SIGHUP, &sa, NULL);

    Config cfg;
    Menu menu;
    memset(&cfg, 0, sizeof(cfg));
    memset(&tree, 0, sizeof(tree));
    memset(&menu, 0, sizeof(menu));

    menu.display = XOpenDisplay(NULL);
    if (!menu.display){
        perror("XOpenDisplay");
        return bye(errno, &cfg, &menu);
    }
    menu.screen = DefaultScreen(menu.display);
    byte code;

    code = config(av[1], &cfg, menu.display, menu.screen);
    if (code != SUCCESS) return bye(code, &cfg, &menu);
    if (stop) return bye(SUCCESS, &cfg, &menu);

    code = init(&menu, &cfg);
    if (code != SUCCESS) return bye(code, &cfg, &menu);
    if (stop) return bye(SUCCESS, &cfg, &menu);

    return bye(update(&menu, &cfg), &cfg, &menu);
}

byte config(const char* const path, Config* const cfg,
            Display* const display, const int screen){
    Colormap colormap = DefaultColormap(display, screen);
    FILE* file = fopen(path, "r");
    if (!file){
        perror("fopen");
        return errno;
    }
    char* line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file) != -1){
        if (line[0] == '#') continue;
        char* key = strtok(line, "=");
        char* value = strtok(NULL, "=");

        if (!strcmp(key, "path")){
            cfg->path = get_realpath(value);
            if (!cfg->path) return errno;
        }
        else if (!strcmp(key, "x"))
            cfg->x = atoi(value);

        else if (!strcmp(key, "y"))
            cfg->y = atoi(value);

        else if (!strcmp(key, "border-size"))
            cfg->border_size = atoi(value);

        else if (!strcmp(key, "border-color")){
            XParseColor(
                display, colormap, value, &cfg->border_color);
            XAllocColor(display, colormap, &cfg->border_color);
        }
        else if (!strcmp(key, "bg-color")){
            XParseColor(
                display, colormap, value, &cfg->bg_color);
            XAllocColor(display, colormap, &cfg->bg_color);
        }
        else if (!strcmp(key, "fg-color")){
            XParseColor(
                display, colormap, value, &cfg->fg_color);
            XAllocColor(display, colormap, &cfg->fg_color);
        }
        else if (!strcmp(key, "focus-bg-color")){
            XParseColor(
                display, colormap, value, &cfg->focus_bg_color);
            XAllocColor(display, colormap, &cfg->focus_bg_color);
        }
        else if (!strcmp(key, "focus-fg-color")){
            XParseColor(
                display, colormap, value, &cfg->focus_fg_color);
            XAllocColor(display, colormap, &cfg->focus_fg_color);
        }
        else if (!strcmp(key, "x-padding"))
            cfg->x_padding = atoi(value);

        else if (!strcmp(key, "y-padding"))
            cfg->y_padding = atoi(value);

        else if (!strcmp(key, "spacing"))
            cfg->spacing = atoi(value);

        else if (!strcmp(key, "font")){
            cfg->font = strdup(value);
            if (!cfg->font) return errno;
        }
        else if (!strcmp(key, "terminal")){
            cfg->terminal = strdup(value);
            if (!cfg->terminal) return errno;
        }
        else if (!strcmp(key, "browser")){
            cfg->browser = strdup(value);
            if (!cfg->browser) return errno;
        }
        else if (!strcmp(key, "search-engine")){
            cfg->search_engine = strdup(value);
            if (!cfg->search_engine) return errno;
        }
        else ft_printf("Unknown key: %s\n", key);
    }
    return SUCCESS;
}

byte bye(const int code, Config* const cfg, Menu* const menu){
    if (cfg->path) free(cfg->path);
    if (cfg->font) free(cfg->font);
    if (cfg->terminal) free(cfg->terminal);
    if (cfg->browser) free(cfg->browser);
    if (cfg->search_engine) free(cfg->search_engine);
    // Here we free the rest
    return code;
}

//int main(void){
//    t_sigaction sa;
//    sa.sa_handler = sig_hdl;
//    sa.sa_flags = 0;
//    sigemptyset(&sa.sa_mask);
//    sigaction(SIGINT, &sa, NULL);
//    sigaction(SIGTERM, &sa, NULL);
//    sigaction(SIGQUIT, &sa, NULL);
//    sigaction(SIGHUP, &sa, NULL);
//
//    Display* const display = XOpenDisplay(NULL);
//    if (!display){
//        perror("XOpenDisplay");
//        exit(errno);
//    };
//    const int screen = DefaultScreen(display);
//
//    t_config* const cfg = config(display, screen);
//    if (!cfg){
//        XCloseDisplay(display);
//        exit(errno);
//    }
//    Window window = XCreateSimpleWindow(
//        display, RootWindow(display, screen),
//        cfg->x, cfg->y, 200, 200, cfg->border_size,
//        cfg->border_color.pixel, cfg->bg_color.pixel);
//    if (!window){
//        free(cfg);
//        XCloseDisplay(display);
//        perror("XCreateSimpleWindow");
//        exit(errno);
//    }
//    GC gc = XCreateGC(display, window, 0, NULL);
//    if (!gc){
//        free(cfg);
//        XCloseDisplay(display);
//        perror("XCreateGC");
//        exit(errno);
//    }
//    XSetForeground(display, gc, cfg->fg_color.pixel);
//    XFontStruct* const font_info = XLoadQueryFont(display, cfg->font);
//    if (!font_info) {
//        free(cfg);
//        XCloseDisplay(display);
//        perror("XLoadQueryFont");
//        exit(errno);
//    }
//    XSetFont(display, gc, font_info->fid);
//    XFreeFont(display, font_info);
//    free(cfg);
//    if (setprops(display, window) == FAILURE){
//        XCloseDisplay(display);
//        exit(errno);
//    }
//    XSelectInput(display, window, ExposureMask | KeyPressMask);
//    XMapWindow(display, window);
//    return update(display, window, gc);
//}
//
//byte setprops(Display* const display, Window window){
//    // ================ Setting window as floating
//    Atom window_type = XInternAtom(
//        display, "_NET_WM_WINDOW_TYPE", False);
//    if (!window_type){
//        perror("XInternAtom");
//        return FAILURE;
//    }
//    Atom window_type_dialog = XInternAtom(
//        display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
//    if (!window_type_dialog){
//        perror("XInternAtom");
//        return FAILURE;
//    }
//    if (!XChangeProperty(
//        display, window, window_type, XA_ATOM, 32,
//        PropModeReplace, (uchar *) &window_type_dialog, 1)){
//        perror("XChangeProperty");
//        return FAILURE;
//    }
//    // ================ Removing system window borders
//    Atom hints = XInternAtom( display, "_MOTIF_WM_HINTS", False);
//    if (!hints){
//        perror("XInternAtom");
//        return FAILURE;
//    }
//    struct{
//        const ulong flags;
//        const ulong functions;
//        const ulong decorations;
//        const long input_mode;
//        const ulong status;
//    }s_hints = {2, 0, 0, 0, 0};
//
//    if (!XChangeProperty(
//        display, window, hints, hints, 32,
//        PropModeReplace, (uchar *)&s_hints, 5)){
//        perror("XChangeProperty");
//        return FAILURE;
//    }
//    return SUCCESS;
//}
