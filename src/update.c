#include "../include/header.h"

extern volatile sig_atomic_t stop;

byte update(Menu* const menu, Config* const cfg){
    (void)cfg;
    XEvent event;
    while (!stop){
        while (XPending(menu->display) > 0){
            XNextEvent(menu->display, &event);
            switch (event.type){
                case Expose:
                    draw(menu);
                    break;
                case KeyPress:
                    return SUCCESS;
                    break;
            }
        }
    }
    return SUCCESS;
}

void draw(Menu* const menu){
    XClearWindow(menu->display, menu->root->window);
    XDrawString(menu->display, menu->root->window, menu->gc,
                10, 10, "Hello, World!", 12);
    XFlush(menu->display);
}
