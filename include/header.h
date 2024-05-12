#ifndef HEADER_H
#define HEADER_H

#define SUCCESS 0
#define FAILURE 1

#define D_FONT "-misc-terminess nerd font propo-medium-r-normal--20-0-0-0-p-0-iso10646-1"
//#define D_FONT "10x20"
#define D_X 20
#define D_Y 20
#define D_BORDER_SIZE 4
#define D_BORDER_R 24
#define D_BORDER_V 24
#define D_BORDER_B 24
#define D_BG_R 12
#define D_BG_V 12
#define D_BG_B 12
#define D_FG_R 255
#define D_FG_V 255
#define D_FG_B 255
#define D_SEL_R 42
#define D_SEL_V 42
#define D_SEL_B 42

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

typedef char byte;
typedef unsigned char ubyte;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef struct sigaction t_sigaction;

typedef struct s_config{
    char* font;
    ushort x;
    ushort y;
    ubyte border_size;
    XColor border_color;
    XColor bg_color;
    XColor fg_color;
} t_config;

t_config* config(Display* const display, const int screen);
byte setprops(Display* const display, Window window);
byte update(Display* const display, Window window, GC gc);

#endif
