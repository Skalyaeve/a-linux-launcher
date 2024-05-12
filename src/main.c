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
    Window window = XCreateSimpleWindow(
        display, RootWindow(display, screen), 10, 10, 200, 200, 1,
        BlackPixel(display, screen), WhitePixel(display, screen));
    if (!window){
        XCloseDisplay(display);
        perror("XCreateSimpleWindow");
        exit(errno);
    }
    if (config(display, window) == FAILURE){
        XCloseDisplay(display);
        exit(errno);
    }
    XMapWindow(display, window);
    return update(display, window, screen);
}

byte config(Display* const display, Window window){
    Atom type = XInternAtom(
        display, "_NET_WM_WINDOW_TYPE", False);
    if (!type){
        perror("XInternAtom");
        return FAILURE;
    }
    Atom type_dialog = XInternAtom(
        display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    if (!type_dialog){
        perror("XInternAtom");
        return FAILURE;
    }
    if (!XChangeProperty(
        display, window, type, XA_ATOM, 32, PropModeReplace,
        (unsigned char *) &type_dialog, 1)){
        perror("XChangeProperty");
        return FAILURE;
    }
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    return SUCCESS;
}

byte update(Display* const display, Window window,
            const int screen){
    XEvent event;
    while (!stop){
        while (XPending(display) > 0){
            XNextEvent(display, &event);
            switch (event.type){
                case Expose:
                    XFillRectangle(
                        display, window,
                        DefaultGC(display, screen),
                        20, 20, 10, 10);
                    break;
                case KeyPress: stop = True;
            }
        }
    }
    XCloseDisplay(display);
    return SUCCESS;
}
