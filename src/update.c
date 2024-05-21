#include "../include/header.h"

extern volatile sig_atomic_t stop;

byte update(Menu* const menu, Config* const cfg){
    XEvent event;
    KeySym keysym;
    byte mode = MENU;
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
                        update_selected(menu, cfg, keysym);
                    else if (keysym == XK_Left
                            || keysym == XK_Right){
                        switch (mode){
                            case MENU:
                                update_focus(menu, cfg, keysym);
                                break;
                            case SEARCH:
                                break;
                        }
                    }
                    else if (keysym == XK_Return){
                        if (menu->focus->selected->exec)
                            exec(menu->focus->selected->exec,
                                 cfg, menu);
                    }
                    else if (keysym == XK_Escape) stop = True;
                    else{
                        if (mode == MENU) mode = SEARCH;
                        if (update_search(menu, cfg, keysym)
                            != SUCCESS) return FAILURE;
                    }
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

void update_selected(Menu* const menu, Config* const cfg,
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

void update_focus(Menu* const menu, Config* const cfg,
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

byte update_search(Menu* const menu, Config* const cfg,
                   const KeySym keysym){
    if (!menu->search && !init_search(menu, cfg))
        return FAILURE;
    Charlist** ptr = &menu->input;
    Charlist* prev = NULL;
    if (keysym != XK_BackSpace){
        while (*ptr){
            prev = *ptr;
            ptr = &(*ptr)->next;
        }
        *ptr = malloc(sizeof(Charlist));
        if (!*ptr){
            perror("malloc");
            return FAILURE;
        }
        (*ptr)->c = XKeysymToKeycode(menu->display, keysym);
        (*ptr)->next = NULL;
        (*ptr)->prev = prev;
        if (prev) prev->next = *ptr;
    }
    else{
    }
    ptr = &menu->input;
    while (*ptr){
        printf("%c", (*ptr)->c);
    }
    printf("\n");
    return SUCCESS;
}

byte init_search(Menu* const menu, Config* const cfg){
    menu->search = malloc(sizeof(Windows));
    if (!menu->search){
        perror("malloc");
        return FAILURE;
    }
    memset(menu->search, 0, sizeof(Windows));
    menu->search->window->x = cfg->x;
    menu->search->window->y = cfg->y;
    menu->search->window->largest = 200;
    menu->search->window->count = 1;
    if (create_search_window(menu, cfg) == FAILURE)
        return FAILURE;
    menu->focus = menu->search;
    return SUCCESS;
}

byte create_search_window(Menu* const menu, Config* const cfg){
    menu->search->window->window = XCreateSimpleWindow(
        menu->display, RootWindow(menu->display, menu->screen),
        menu->search->window->x, menu->search->window->y,
        menu->search->largest + cfg->x_padding * 2,
        cfg->font_size * menu->search->count
        + cfg->line_margin * (menu->search->count - 1)
        + cfg->y_padding * 2, cfg->border_size,
        cfg->border_color.pixel, cfg->bg_color.pixel);
    if (setwindows(menu->display, menu->search) == FAILURE)
        return FAILURE;
    return SUCCESS;
}
