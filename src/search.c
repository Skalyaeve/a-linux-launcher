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
    menu->search->input->count = 1;

    menu->search->result = malloc(sizeof(Windows));
    if (!menu->search->result){
        perror("malloc");
        return FAILURE;
    }
    memset(menu->search->result, 0, sizeof(Windows));
    menu->search->result->x = cfg->x;
    menu->search->result->y = cfg->y + cfg->font_size
        + cfg->y_padding * 2 + cfg->window_margin;
    menu->search->result->visible = YES;

    menu->search->result->entries = malloc(sizeof(Entry));
    if (!menu->search->result->entries){
        perror("malloc");
        return FAILURE;
    }
    memset(menu->search->result->entries, 0, sizeof(Entry));
    menu->search->result->entries->fullname = strdup("Web Search: ");
    if (!menu->search->result->entries->fullname){
        perror("strdup");
        return FAILURE;
    }
    menu->search->result->entries->name
        = strndup("Web Search: ", cfg->max_len);
    if (!menu->search->result->entries->name){
        perror("strndup");
        return FAILURE;
    }
    menu->search->result->entries->exec
        = malloc(strlen(cfg->browser)
                 + strlen(cfg->search_engine) + 2);
    if (!menu->search->result->entries->exec){
        perror("malloc");
        return FAILURE;
    }
    snprintf(menu->search->result->entries->exec,
             strlen(cfg->browser) + strlen(cfg->search_engine) + 2,
             "%s %s", cfg->browser, cfg->search_engine);

    menu->search->result->draw_start = menu->search->result->entries;
    menu->search->result->draw_end = menu->search->result->entries;
    return SUCCESS;
}

byte update_search(Menu* const menu, const KeySym keysym,
                   byte* const mode, Config* const cfg){
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
        (*ptr)->len++;
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
        (*ptr)->len--;
        if (!prev){
            *mode = MENU;
            toggle_menu(YES, menu->display, menu->root);
            toggle_search(NO, menu);
            return SUCCESS;
        }
        prev->next = NULL;
    }
    char* const str = update_input(menu, cfg);
    if (!str) return FAILURE;
    if (update_result(menu, cfg, str) == FAILURE){
        free(str);
        return FAILURE;
    }
    free(str);
    return SUCCESS;
}

char* update_input(Menu* const menu, Config* const cfg){
    char* const str = malloc(menu->search->len + 1);
    if (!str){
        perror("malloc");
        return NULL;
    }
    size_t x = 0;
    for (Chardlist* ptr = menu->search->str; ptr;
        ptr = ptr->next, x++)
        str[x] = ptr->c;
    str[x] = '\0';

    Windows* const input = menu->search->input;
    input->largest = x * cfg->font_size / 2 + cfg->x_padding * 2;
    if (input->window) XDestroyWindow(menu->display, input->window);

    input->window = new_window(input, menu, cfg);
    if (!input->window
        || setwindows(menu->display, input) == FAILURE){
        free(str);
        return NULL;
    }
    XMapWindow(menu->display, input->window);
    XWindowAttributes attr;
    do{
        XGetWindowAttributes(menu->display, input->window, &attr);
    }while (attr.map_state != IsViewable);

    XSetForeground(menu->display, menu->gc,
                   cfg->input_bg_color.pixel);
    XFillRectangle(menu->display, input->window, menu->gc,
                   cfg->x_padding, cfg->y_padding,
                   window->largest - cfg->x_padding * 2,
                   cfg->font_size - cfg->y_padding * 2);
    XSetForeground(menu->display, menu->gc,
                   cfg->input_fg_color.pixel);
    XDrawString(menu->display, input->window, menu->gc,
                cfg->x_padding, cfg->y_padding + cfg->font_size,
                str, x);
    return str;
}

byte update_result(Menu* const menu, Config* const cfg,
                   char* const str){
    Windows* const result = menu->search->result;
    if (result->entries) free_entries(result->entries);
    result->entries = get_search_result(menu->root->entries,
                                        str, menu, cfg);
    if (!result->entries) return FAILURE;

    if (result->window)
        XDestroyWindow(menu->display, result->window);
    result->window = new_window(result, menu, cfg);
    if (!result->window
        || setwindows(menu->display, result) == FAILURE){
        free_entries(result->entries);
        return FAILURE;
    }
    XMapWindow(menu->display, result->window);
    XWindowAttributes attr;
    do{
        XGetWindowAttributes(menu->display, result->window, &attr);
    }while (attr.map_state != IsViewable);
    return SUCCESS;
}

Entry* get_search_result(Entry* src, char* const str){
    Entry* result = NULL;
    Entry* prev = NULL;
    Entry** ptr = &result;
    while (src){
        if (src->have_child == YES){
            *ptr = get_search_result(src->child, str);
            if (!*ptr){
                free_entries(result);
                return NULL;
            }
            (*ptr)->prev = prev;
            while ((*ptr)->next) *ptr = (*ptr)->next;
            ptr = &(*ptr)->next;
        }
        else if (strstr(src->fullname, str)){
            *ptr = malloc(sizeof(Entry));
            if (!*ptr){
                perror("malloc");
                free_entries(result);
                return NULL;
            }
            memset(*ptr, 0, sizeof(Entry));
            (*ptr)->fullname = strdup(src->fullname);
            if (!(*ptr)->fullname){
                perror("strdup");
                free_entries(result);
                return NULL;
            }
            (*ptr)->name = strdup(src->name);
            if (!(*ptr)->name){
                perror("strdup");
                free_entries(result);
                return NULL;
            }
            (*ptr)->exec = strdup(src->exec);
            if (!(*ptr)->exec){
                perror("strdup");
                free_entries(result);
                return NULL;
            }
            (*ptr)->prev = prev;
            ptr = &(*ptr)->next;
        }
        src = src->next;
    }
    return result;
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
