
#ifndef __ORPHEUS_ORPHEUS_H__
#define __ORPHEUS_ORPHEUS_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include <string.h>

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/XKBlib.h>

#define LENGTH(X) (sizeof X / sizeof X[0])
#define EMOJI_SET(X) { X, LENGTH(X), 0 }
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + lrpad)
#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B) ((A) <= (X) && (X) <= (B))

#if defined(__GNUC__) || defined(__clang__)
    #define UNUSED(name) _unused_ ## name __attribute__((unused))
#else
    #define UNUSED(name) _unused_ ## name
#endif

#define UTF_INVALID 0xFFFD
#define UTF_SIZ 4

typedef union Emoji {
    uint64_t raw;
    struct {
        uint32_t id;
        uint16_t group;
        uint16_t expand;
    };
} Emoji;

typedef struct {
    Emoji *set;
    size_t length;
    uint32_t max_scroll;
} EmojiSet;


void panic(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);

#endif // __ORPHEUS_ORPHEUS_H__
