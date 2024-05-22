#include "../include/header.h"

extern volatile sig_atomic_t stop;

byte init_search(Menu* const menu, Config* const cfg){
    menu->search = malloc(sizeof(Input));
    if (!menu->search){
        perror("malloc");
        return FAILURE;
    }
    memset(menu->search, 0, sizeof(Input));

    menu->search->input = malloc(sizeof(Windows));
    if (!menu->search->input){
        perror("malloc");
        return FAILURE;
    }
    memset(menu->search->input, 0, sizeof(Windows));
    menu->search->input->x = cfg->x;
    menu->search->input->y = cfg->y;
    menu->search->input->largest = 200;
    menu->search->input->count = 1;
    menu->search->input->window = new_window(
        menu->search->input, menu, cfg);
    if (!menu->search->input->window) return FAILURE;
    if (setwindows(menu->display, menu->search->input) == FAILURE){
        perror("setwindows");
        return FAILURE;
    }
    menu->search->result = malloc(sizeof(Windows));
    if (!menu->search->result){
        perror("malloc");
        return FAILURE;
    }
    memset(menu->search->result, 0, sizeof(Windows));
    menu->search->result->x = cfg->x;
    menu->search->result->y = cfg->y + cfg->font_size
        + cfg->y_padding * 2 + cfg->window_margin;
    menu->search->result->largest = 200;
    menu->search->result->count = 1;
    menu->search->result->window = new_window(
        menu->search->result, menu, cfg);
    if (!menu->search->result->window) return FAILURE;
    if (setwindows(menu->display, menu->search->result) == FAILURE){
        perror("setwindows");
        return FAILURE;
    }
    menu->search->input->visible = YES;
    menu->search->result->visible = YES;
    return SUCCESS;
}

void toggle_search(const bool show, Menu* const menu){
    if (show){
        menu->focus = menu->search->result;
        menu->active = menu->search->result;
        XMoveWindow(menu->display, menu->search->input->window,
                    menu->search->input->x,
                    menu->search->input->y);
        XMapWindow(menu->display, menu->search->input->window);
        XMoveWindow(menu->display, menu->search->result->window,
                    menu->search->result->x,
                    menu->search->result->y);
        XMapWindow(menu->display, menu->search->result->window);
    }
    else{
        menu->focus = menu->last_focus;
        menu->active = menu->root;
        XUnmapWindow(menu->display, menu->search->input->window);
        XUnmapWindow(menu->display, menu->search->result->window);
    }
}

byte update_search(Menu* const menu, const KeySym keysym,
                   byte* const mode){
    Chardlist** ptr = &menu->search->str;
    Chardlist* prev = NULL;
    if (keysym != XK_BackSpace){
        while (*ptr){
            prev = *ptr;
            ptr = &(*ptr)->next;
        }
        *ptr = malloc(sizeof(Chardlist));
        if (!*ptr){
            perror("malloc");
            return FAILURE;
        }
        (*ptr)->c = XKeysymToString(keysym)[0];
        (*ptr)->next = NULL;
        (*ptr)->prev = prev;
        if (prev) prev->next = *ptr;
    }
    else{
        while ((*ptr)->next){
            prev = *ptr;
            ptr = &(*ptr)->next;
        }
        free(*ptr);
        *ptr = NULL;
        if (!prev){
            *mode = MENU;
            toggle_menu(YES, menu->display, menu->root);
            toggle_search(NO, menu);
            return SUCCESS;
        }
        prev->next = NULL;
    }
    ptr = &menu->search->str;
    while (*ptr){
        printf("%c", (*ptr)->c);
        ptr = &(*ptr)->next;
    }
    printf("\n");
    return SUCCESS;
}
