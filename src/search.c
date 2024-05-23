#include "../include/header.h"

extern volatile sig_atomic_t stop;

byte init_search(Menu* const menu, Config* const cfg){
    menu->search = malloc(sizeof(Input));
    if (!menu->search){
        perror("malloc");
        return FAILURE;
    }
    memset(menu->search, 0, sizeof(Input));

    menu->search->result = malloc(sizeof(Windows));
    if (!menu->search->result){
        perror("malloc");
        return FAILURE;
    }
    memset(menu->search->result, 0, sizeof(Windows));
    menu->search->result->x = cfg->x;
    menu->search->result->y = cfg->y;
    menu->search->result->visible = YES;
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
        menu->search->len++;
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
        menu->search->len--;
        if (!prev){
            *mode = MENU;
            toggle_menu(YES, menu->display, menu->root);
            menu->focus = menu->last_focus;
            menu->active = menu->root;
            XUnmapWindow(menu->display,
                         menu->search->result->window);
            return SUCCESS;
        }
        prev->next = NULL;
    }
    char* const str = update_input(menu);
    if (!str) return FAILURE;
    if (update_result(menu, cfg, str) == FAILURE){
        free(str);
        return FAILURE;
    }
    free(str);
    menu->search->result->selected = menu->search->result->entries;
    menu->search->result->index = 1;
    return SUCCESS;
}

char* update_input(Menu* const menu){
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
    return str;
}

byte update_result(Menu* const menu, Config* const cfg,
                   char* const str){
    Windows* const result = menu->search->result;
    if (result->entries) free_entries(result->entries);
    result->largest = 0;
    result->count = 0;
    result->entries = get_search_result(menu->root->entries,
                                        str, cfg, menu);
    if (add_websearch(menu, cfg, str) != SUCCESS) return FAILURE;

    result->largest = result->largest * cfg->font_size / 2
        + cfg->x_padding * 2;

    Window tmp_window = new_window(result, menu, cfg);
    if (!tmp_window ||
        setwindows(menu->display, result, &tmp_window) == FAILURE){
        free_entries(result->entries);
        return FAILURE;
    }
    XMapWindow(menu->display, tmp_window);
    XWindowAttributes attr;
    do{
        XGetWindowAttributes(menu->display, tmp_window, &attr);
    }while (attr.map_state != IsViewable);
    if (result->window)
        XDestroyWindow(menu->display, result->window);
    result->window = tmp_window;
    return SUCCESS;
}

Entry* get_search_result(Entry* src, char* const str,
                         Config* const cfg, Menu* const menu){
    Entry* result = NULL;
    Entry* prev = NULL;
    Entry** ptr = &result;
    ushort size;
    while (src){
        if (src->have_child == YES){
            *ptr = get_search_result(src->child->entries,
                                     str, cfg, menu);
            while (*ptr){
                size = strlen((*ptr)->name);
                if (size > menu->search->result->largest)
                    menu->search->result->largest = size;
                (*ptr)->prev = prev;
                prev = *ptr;
                ptr = &(*ptr)->next;
            }
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
            size = strlen((*ptr)->name);
            if (size > menu->search->result->largest)
                menu->search->result->largest = size;
            (*ptr)->prev = prev;
            ptr = &(*ptr)->next;
            menu->search->result->count++;
        }
        src = src->next;
    }
    return result;
}

byte add_websearch(Menu* const menu, Config* const cfg,
                   char* const str){
    Entry* ptr = menu->search->result->entries;
    if (ptr){
        while (ptr->next) ptr = ptr->next;
        ptr->next = malloc(sizeof(Entry));
        if (!ptr->next){
            perror("malloc");
            return FAILURE;
        }
        memset(ptr->next, 0, sizeof(Entry));
        ptr->next->prev = ptr;
        ptr = ptr->next;
    }
    else{
        ptr = malloc(sizeof(Entry));
        if (!ptr){
            perror("malloc");
            return FAILURE;
        }
        memset(ptr, 0, sizeof(Entry));
        menu->search->result->entries = ptr;
    }
    size_t len = strlen(str) + 13;
    ptr->fullname = malloc(len);
    if (!ptr->fullname){
        perror("malloc");
        return FAILURE;
    }
    snprintf(ptr->fullname, len, "web search: %s", str);
    ptr->name = strndup(ptr->fullname, cfg->max_len);
    if (!ptr->name){
        perror("strdup");
        return FAILURE;
    }
    len = strlen(cfg->browser) + strlen(cfg->search_engine)
        + strlen(str) + 2;
    ptr->exec = malloc(len);
    if (!ptr->exec){
        perror("malloc");
        return FAILURE;
    }
    snprintf(ptr->exec, len, "%s %s%s", cfg->browser,
             cfg->search_engine, str);
    menu->search->result->count++;
    const ushort size = strlen(ptr->name);
    if (size > menu->search->result->largest)
        menu->search->result->largest = size;
    return SUCCESS;
}
