#ifndef HEADER_H
#define HEADER_H

#define SUCCESS 0
#define FAILURE 1

typedef char byte;
typedef unsigned char ubyte;
typedef unsigned char bool;
typedef struct sigaction t_sigaction;

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

byte config(Display* const display, Window window);
byte update(Display* const display, Window window,
            const int screen);
#endif
