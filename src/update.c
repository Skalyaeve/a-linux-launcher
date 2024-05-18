#include "../include/header.h"

extern volatile sig_atomic_t stop;

byte update(Menu* const menu, Config* const cfg){
    XEvent event;
    KeySym keysym;
    while (!stop){
        while (XPending(menu->display) > 0){
            XNextEvent(menu->display, &event);
            switch (event.type){
                case Expose:
                    draw(cfg, menu, menu->root);
                    break;
                case KeyPress:
                    keysym = XLookupKeysym(&event.xkey, 0);
                    if (keysym == XK_Up || keysym == XK_Down)
                        change_selected(menu, cfg, keysym);
                    else if (keysym == XK_Left || keysym == XK_Right)
                        change_focus(menu, cfg, keysym);
                    else if (keysym == XK_Return){
                        if (menu->focus->selected->exec)
                            exec(menu->focus->selected->exec,
                                 cfg, menu);
                    }
                    else if (keysym == XK_Escape) stop = True;
                    break;
            }
        }
    }
    return SUCCESS;
}

void draw(Config* const cfg, Menu* const menu,
          Windows* const window){
    if (window->visible == NO) return;
    Entry* ptr = window->entries;
    ushort y = cfg->y_padding + cfg->font_size * 0.75;
    while (ptr){
        XSetForeground(menu->display, menu->gc, cfg->bg_color.pixel);
        XFillRectangle(menu->display, window->window, menu->gc,
                       0, y - cfg->font_size - cfg->font_size * 0.25,
                       cfg->x_padding * 2 + window->largest + 100,
                       cfg->font_size + cfg->line_margin);
        if (ptr == window->selected){
            XSetForeground(menu->display, menu->gc,
                           cfg->focus_bg_color.pixel);
            XFillRectangle(menu->display, window->window, menu->gc,
                           0, y - cfg->font_size - cfg->font_size
                           * 0.25, cfg->x_padding * 2 + 100
                           + window->largest, cfg->font_size
                           + cfg->line_margin);
            XSetForeground(menu->display, menu->gc,
                           cfg->focus_fg_color.pixel);
        }
        else XSetForeground(menu->display, menu->gc,
                            cfg->fg_color.pixel);
        XDrawString(menu->display, window->window, menu->gc,
                    cfg->x_padding, y, ptr->name, strlen(ptr->name));
        y += cfg->font_size + cfg->line_margin;
        if (ptr->child) draw(cfg, menu, ptr->child);
        ptr = ptr->next;
    }
}

void change_selected(Menu* const menu, Config* const cfg,
                     const KeySym keysym){
    Entry* ptr;
    if (!menu->focus->selected)
        menu->focus->selected = menu->focus->entries;
    else{
        if (menu->focus->selected->child){
            menu->focus->selected->child->visible = NO;
            XUnmapWindow(menu->display,
                         menu->focus->selected->child->window);
        }
        switch (keysym){
            case XK_Up:
                menu->focus->selected = menu->focus->selected->prev;
                if (!menu->focus->selected){
                    ptr = menu->focus->entries;
                    while (ptr->next) ptr = ptr->next;
                    menu->focus->selected = ptr;
                }
                break;
            case XK_Down:
                menu->focus->selected = menu->focus->selected->next;
                if (!menu->focus->selected)
                    menu->focus->selected = menu->focus->entries;
                break;
        }
    }
    if (menu->focus->selected->child) spawn_child(menu);
    draw(cfg, menu, menu->root);
}

void change_focus(Menu* const menu, Config* const cfg,
                  const KeySym keysym){
    if (!menu->focus->selected) return;
    switch (keysym){
        case XK_Left:
            if (!menu->focus->parent) return;
            if (menu->focus->selected->child){
                menu->focus->selected->child->visible = NO;
                XUnmapWindow(menu->display,
                             menu->focus->selected->child->window);
            }
            menu->focus->selected = NULL;
            menu->focus = menu->focus->parent;
            break;
        case XK_Right:
            if (!menu->focus->selected->child) return;
            menu->focus = menu->focus->selected->child;
            menu->focus->selected = menu->focus->entries;
            if (menu->focus->selected->child) spawn_child(menu);
            break;
    }
    draw(cfg, menu, menu->root);
}

void spawn_child(Menu* const menu){
    XWindowAttributes attr;
    menu->focus->selected->child->visible = YES;
    XMoveWindow(menu->display, menu->focus->selected->child->window,
                menu->focus->selected->child->x,
                menu->focus->selected->child->y);
    XMapWindow(menu->display, menu->focus->selected->child->window);
    do{
        XGetWindowAttributes(
            menu->display,
            menu->focus->selected->child->window, &attr);
    }while (attr.map_state != IsViewable);
    XSetInputFocus(menu->display, menu->focus->window,
                   RevertToParent, CurrentTime);
}

void exec(char* const cmd, Config* const cfg, Menu* const menu){
    char* const tmp = strdup(cmd);
    if (!tmp){
        perror("strdup");
        exit(bye(errno, cfg, menu));
    }
    bye(SUCCESS, cfg, menu);
    pid_t pid = fork();
    if (pid == -1){
        free(tmp);
        perror("fork");
        exit(errno);
    }
    if (!pid){
        system(tmp);
        free(tmp);
        exit(SUCCESS);
    }
    free(tmp);
    exit(SUCCESS);
}
