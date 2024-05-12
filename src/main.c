#include "../include/header.h"

volatile sig_atomic_t stop = False;
void sig_hdl(int sig){ (void)sig; stop = True; }

int main(void){
    t_sigaction sa;
    sa.sa_handler = sig_hdl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    Display* const display = XOpenDisplay(NULL);
    if (!display){
        perror("XOpenDisplay");
        exit(errno);
    };
    const int screen = DefaultScreen(display);

    t_config* const cfg = config(display, screen);
    if (!cfg){
        XCloseDisplay(display);
        exit(errno);
    }
    Window window = XCreateSimpleWindow(
        display, RootWindow(display, screen),
        cfg->x, cfg->y, 200, 200, cfg->border_size,
        cfg->border_color.pixel, cfg->bg_color.pixel);
    if (!window){
        free(cfg);
        XCloseDisplay(display);
        perror("XCreateSimpleWindow");
        exit(errno);
    }
    GC gc = XCreateGC(display, window, 0, NULL);
    if (!gc){
        free(cfg);
        XCloseDisplay(display);
        perror("XCreateGC");
        exit(errno);
    }
    XSetForeground(display, gc, cfg->fg_color.pixel);
    XFontStruct* const font_info = XLoadQueryFont(display, cfg->font);
    if (!font_info) {
        free(cfg);
        XCloseDisplay(display);
        perror("XLoadQueryFont");
        exit(errno);
    }
    XSetFont(display, gc, font_info->fid);
    XFreeFont(display, font_info);
    free(cfg);
    if (setprops(display, window) == FAILURE){
        XCloseDisplay(display);
        exit(errno);
    }
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    return update(display, window, gc);
}

t_config* config(Display* const display, const int screen){
    Colormap colormap = DefaultColormap(display, screen);
    t_config* const cfg = malloc(sizeof(t_config));
    if (!cfg){
        perror("malloc");
        return NULL;
    }
    cfg->font = D_FONT;
    cfg->x = D_X;
    cfg->y = D_Y;
    cfg->border_size = D_BORDER_SIZE;

    cfg->border_color.red = D_BORDER_R * 257;
    cfg->border_color.green = D_BORDER_V * 257;
    cfg->border_color.blue = D_BORDER_B * 257;
    XAllocColor(display, colormap, &cfg->border_color);

    cfg->bg_color.red = D_BG_R * 257;
    cfg->bg_color.green = D_BG_V * 257;
    cfg->bg_color.blue = D_BG_B * 257;
    XAllocColor(display, colormap, &cfg->bg_color);

    cfg->fg_color.red = D_FG_R * 257;
    cfg->fg_color.green = D_FG_V * 257;
    cfg->fg_color.blue = D_FG_B * 257;
    XAllocColor(display, colormap, &cfg->fg_color);
    return cfg;
}

byte setprops(Display* const display, Window window){
    // ================ Setting window as floating
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
        display, window, window_type, XA_ATOM, 32,
        PropModeReplace, (uchar *) &window_type_dialog, 1)){
        perror("XChangeProperty");
        return FAILURE;
    }
    // ================ Removing system window borders
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
        display, window, hints, hints, 32,
        PropModeReplace, (uchar *)&s_hints, 5)){
        perror("XChangeProperty");
        return FAILURE;
    }
    return SUCCESS;
}
