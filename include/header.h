#ifndef HEADER_H
#define HEADER_H

#define SUCCESS 0
#define FAILURE 1

#define YES 1
#define NO 0

#define APP 1
#define LINK 2

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
typedef struct s_exec Exec;
typedef struct s_intlist Intlist;

struct s_config{
    char* path;
    ushort x;
    ushort y;
    ubyte border_size;
    XColor border_color;
    XColor bg_color;
    XColor fg_color;
    XColor focus_bg_color;
    XColor focus_fg_color;
    ushort x_padding;
    ushort y_padding;
    ushort line_margin;
    ushort window_margin;
    ushort font_size;
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
};
struct s_windows{
    ushort x;
    ushort y;
    bool visible;
    Window window;
    size_t largest;
    Entry* entries;
    Entry* selected;
    Windows* parent;
};
struct s_entry{
    char* name;
    char* exec;
    Windows* child;
    Entry* next;
    Entry* prev;
};
struct s_exec{
    ushort type;
    char* cmd;
    bool terminal;
};
struct s_intlist{
    int value;
    Intlist* next;
};

//================================ MAIN
void sig_hdl(const int sig);
int config(const char* const path, Config* const cfg,
            Display* const display, const int screen);
int init(Menu* const menu, Config* const cfg);
byte setwindows(Display* const display, Windows* const ptr);
byte setprops(Display* const display, Window* const window);
byte bye(const int code, Config* const cfg, Menu* const menu);

//================================ WINDOW
Windows* create_window(const char* const path,
                       Menu* const menu, Config* const cfg,
                       size_t x_offset, size_t y_offset);
void link_entries(Entry* entries);
bool create_childs(Windows* const window, Intlist* listptr,
                   const char* const path, const size_t largest,
                   size_t x_offset, size_t y_offset,
                   Menu* const menu, Config* const cfg);
void free_window(Windows* const window);
int new_app(const char* const path, Entry* const entry,
            Config* const cfg);
char* fill_exec(Exec* const exec, Config* const cfg);

//================================ UPDATE
byte update(Menu* const menu, Config* const cfg);
void draw(Config* const cfg, Menu* const menu,
          Windows* const window);
void change_selected(Menu* const menu, Config* const cfg,
                     const KeySym keysym);
void change_focus(Menu* const menu, Config* const cfg,
                  const KeySym keysym);
void spawn_child(Menu* const menu);

//================================ UTILS
char* get_realpath(const char* const path);
void free_intlist(Intlist* list);

#endif
