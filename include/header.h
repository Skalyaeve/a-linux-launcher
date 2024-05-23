#ifndef HEADER_H
#define HEADER_H

#define SUCCESS 0
#define FAILURE 1

#define YES 1
#define NO 0

#define APP 1
#define LINK 2

#define MENU 1
#define SEARCH 2

#define LOCKFILE "/tmp/launcher.ft.lock"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

typedef char bool;
typedef char byte;
typedef unsigned char ubyte;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef struct sigaction Sigaction;
typedef struct stat Stat;
typedef struct dirent Dirent;

typedef struct s_config Config;
typedef struct s_entry Entry;
typedef struct s_windows Windows;
typedef struct s_menu Menu;
typedef struct s_input Input;
typedef struct s_exec Exec;
typedef struct s_strlist Strlist;
typedef struct s_chardlist Chardlist;

struct s_config{
    char* path;
    short x;
    short y;
    ubyte border_size;
    XColor border_color;
    XColor bg_color;
    XColor fg_color;
    XColor focus_bg_color;
    XColor focus_fg_color;
    XColor input_bg_color;
    XColor input_fg_color;
    short x_padding;
    short y_padding;
    short line_margin;
    short window_margin;
    ushort font_size;
    ushort max_len;
    ushort max_lines;
    char* font;
    char* terminal;
    char* shell;
    char* browser;
    char* search_engine;
};
struct s_menu{
    Display* display;
    int screen;
    GC gc;
    Windows* root;
    Windows* focus;
    Windows* last_focus;
    Windows* active;
    Input* search;
};
struct s_input{
    Chardlist* str;
    size_t len;
    Windows* result;
};
struct s_windows{
    short x;
    short y;
    ushort largest;
    ushort count;
    ushort index;
    Window window;
    bool visible;
    Entry* entries;
    Entry* selected;
    Entry* draw_start;
    Entry* draw_end;
    Windows* parent;
};
struct s_entry{
    char* name;
    char* fullname;
    char* exec;
    bool have_child;
    Windows* child;
    Entry* next;
    Entry* prev;
};
struct s_exec{
    ushort type;
    char* cmd;
    bool terminal;
};
struct s_strlist{
    char* str;
    Strlist* next;
};
struct s_chardlist{
    char c;
    Chardlist* next;
    Chardlist* prev;
};

//================================ MAIN
void sig_hdl(const int sig);
int config(const char* const path, Config* const cfg,
            Display* const display, const int screen);
int check_args(Config* const cfg);
int init(Menu* const menu, Config* const cfg);
byte setwindows(Display* const display, Windows* const ptr,
                Window* const window);
byte setprops(Display* const display, Window* const window);
byte bye(const int code, Config* const cfg, Menu* const menu);

//================================ WINDOW
Windows* create_window(const char* const path,
                       Menu* const menu, Config* const cfg,
                       ushort x_offset, ushort y_offset);
void link_entries(Entry* entries);
bool create_childs(Windows* const window, const char* const path,
                   const ushort largest, ushort x_offset,
                   ushort y_offset, Menu* const menu,
                   Config* const cfg);
void free_window(Windows* const window);
void free_entries(Entry* entries);
int new_app(const char* const path, Entry* const entry,
            Config* const cfg);
char* fill_exec(Exec* const exec, Config* const cfg);
Strlist* get_order(const char* const path);
byte order_entries(Entry* entries, Strlist* order,
                   const char* const path);
Window new_window(Windows* const window, Menu* const menu,
                    Config* const cfg);

//================================ UPDATE
byte update(Menu* const menu, Config* const cfg);
void draw(Config* const cfg, Menu* const menu,
          Windows* const window);
void update_selected(Menu* const menu, Config* const cfg,
                     const KeySym keysym);
void update_focus(Menu* const menu, Config* const cfg,
                  const KeySym keysym);
void spawn_child(Menu* const menu);
void exec(char* const cmd, Config* const cfg, Menu* const menu);
void toggle_menu(const bool show, Display* const display,
                 Windows* const window);

//================================ SEARCH
byte init_search(Menu* const menu, Config* const cfg);
byte update_search(Menu* const menu, const KeySym keysym,
                   byte* const mode, Config* const cfg);
char* update_input(Menu* const menu);
byte update_result(Menu* const menu, Config* const cfg,
                   char* const str);
Entry* get_search_result(Entry* src, char* const str,
                         Config* const cfg, Menu* const menu);
byte add_websearch(Menu* const menu, Config* const cfg,
                   char* const str);

//================================ UTILS
char* get_realpath(const char* const path);
void free_strlist(Strlist* list);
void free_chardlist(Chardlist* list);
void ft_strip(char** str);

#endif
