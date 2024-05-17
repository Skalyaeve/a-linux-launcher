#include "../include/header.h"

extern volatile sig_atomic_t stop;

byte update(Menu* const menu, Config* const cfg){
    XEvent event;
    while (!stop){
        while (XPending(menu->display) > 0){
            XNextEvent(menu->display, &event);
            switch (event.type){
                case Expose:
                    draw(cfg, menu, menu->root);
                    break;
                case KeyPress:
                    return SUCCESS;
                    break;
            }
        }
    }
    return SUCCESS;
}

void draw(Config* const cfg, Menu* const menu, Windows* window){
    if(window->visible){
        Entry* currentEntry = window->entries;
        ushort y = cfg->y_padding + cfg->font_size * 0.75;
        while(currentEntry != NULL){

            XDrawString(menu->display, window->window, menu->gc,
                        cfg->x_padding, y, currentEntry->name,
                        strlen(currentEntry->name));
            y += cfg->font_size + cfg->spacing;

            if(currentEntry->child != NULL)
                draw(cfg, menu, currentEntry->child);
            currentEntry = currentEntry->next;
        }
    }
}
