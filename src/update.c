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
}
