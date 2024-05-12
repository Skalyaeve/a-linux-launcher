#include "../include/header.h"
#include <X11/keysym.h>
#include <stdlib.h>

extern volatile sig_atomic_t stop;

typedef struct Menu {
    char** entries;
    int num_entries;
    int selected_entry;
} Menu;

char* main_entries[] = {
    "Entry 1 >",
    "Entry 2",
    "Entry 3",
    "Entry 4 >",
    "Entry 5 >"
};
char* sub1[] = {
    "sub 1",
    "sub 2",
};
char* sub4[] = {
    "sub 1",
};
char* sub5[] = {
    "sub 1",
    "sub 2",
    "sub 3",
};

Menu menus[] = {
    {main_entries, sizeof(main_entries) / sizeof(main_entries[0]), 0},
    {sub1, sizeof(sub1) / sizeof(sub1[0]), 0},
    {NULL, 0, 0},
    {sub4, sizeof(sub4) / sizeof(sub4[0]), 0},
    {sub5, sizeof(sub5) / sizeof(sub5[0]), 0}
};

Menu* menu_stack[10];
int top = 0;

void push(Menu* menu) {
    menu_stack[top++] = menu;
}

Menu* pop() {
    return menu_stack[--top];
}

Menu* peek() {
    return menu_stack[top - 1];
}

void draw_menu(Display* const display, Window window, GC gc, XColor color, Menu* menu){
    for (int i = 0; i < menu->num_entries; i++){
        if (i == menu->selected_entry){
            XSetForeground(display, gc, color.pixel);
            XFillRectangle(display, window, gc, 0, 32 * i, 200, 32);
            XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
        }
        XDrawString(
            display, window, gc, 16, 32 * i + 28,
            menu->entries[i], strlen(menu->entries[i]));
        if (i == menu->selected_entry){
            XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
        }
    }
}

byte update(Display* const display, Window window, GC gc){
    XEvent event;
    XColor color;
    Colormap colormap;
    char green[] = "#00FF00";

    colormap = DefaultColormap(display, 0);
    XParseColor(display, colormap, green, &color);
    XAllocColor(display, colormap, &color);

    XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));

    push(&menus[0]);

    while (!stop){
        while (XPending(display) > 0){
            XNextEvent(display, &event);
            switch (event.type){
                case Expose:
                    draw_menu(display, window, gc, color, peek());
                    break;
                case KeyPress:
                    if (event.xkey.keycode == XKeysymToKeycode(display, XK_Up)){
                        peek()->selected_entry = (peek()->selected_entry - 1 + peek()->num_entries) % peek()->num_entries;
                    } else if (event.xkey.keycode == XKeysymToKeycode(display, XK_Down)){
                        peek()->selected_entry = (peek()->selected_entry + 1) % peek()->num_entries;
                    } else if (event.xkey.keycode == XKeysymToKeycode(display, XK_Right)){
                        if (peek()->entries[peek()->selected_entry][strlen(peek()->entries[peek()->selected_entry]) - 1] == '>'){
                            push(&menus[peek()->selected_entry + 1]);
                        }
                    } else if (event.xkey.keycode == XKeysymToKeycode(display, XK_Left)){
                        if (top > 1){
                            pop();
                        }
                    }
                    XClearWindow(display, window);
                    draw_menu(display, window, gc, color, peek());
                    break;
            }
        }
    }
    XFreeGC(display, gc);
    XCloseDisplay(display);
    return SUCCESS;
}
