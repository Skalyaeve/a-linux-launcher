#ifndef HEADER_H
#define HEADER_H

#define SUCCESS 0
#define FAILURE 1

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

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
    ushort spacing;
    char* font;
    char* terminal;
    char* browser;
    char* search_engine;
};

struct s_entry{
    char* name;
    char* exec;
    Windows* child;
    Entry* next;
};

struct s_windows{
    Window window;
    Entry* entries;
};

struct s_menu{
    Display* display;
    int screen;
    GC gc;
    Windows* root;
};

#endif
