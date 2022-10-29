#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <stdlib.h>

#include "drw.h"
#include "util.h"

#define LENGTH(X) (sizeof X / sizeof X[0])
#define EMOJI_SET(X) { X, LENGTH(X) }
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + lrpad)

typedef struct {
    const char **emojis;
    int length;
} EmojiSet;


static void run(void);
static void setup(void);

// events
static void expose(XEvent *e);
static void buttonpress(XEvent *e);
static void keypress(XEvent *e);
static void mousemove(XEvent *e);
static void window_leave(XEvent *e);

// draws
static void draw_tabs(void);
static void draw_grid(void);
static void draw_emoji(int c, int r);

// utils
int get_block(int x, int y, int *c, int *r);
int is_emoji(int c, int r);
void update_emoji_focus(int c, int r);


/* variables */
static void (*handler[LASTEvent])(XEvent *) = {
    [ButtonPress] = buttonpress,
    [Expose] = expose,
    [MotionNotify] = mousemove,
    [KeyPress] = keypress,
    [LeaveNotify] = window_leave,
};
static int running = 1;
static int screen;
static Display *dpy;
static Window win;
static Drw *drw;
static int lrpad;
static int scroll = 0;
static Cursor c_hover;
// current emoji pos
static int crnt_emoji_c = -1;
static int crnt_emoji_r = -1;
// last emoji pos
static int last_emoji_c = -1;
static int last_emoji_r = -1;

#include "config.h"

void run(void) {
    XEvent ev;
    /* main event loop */
    XSync(dpy, False);
    while (running && !XNextEvent(dpy, &ev))
        if (handler[ev.type])
            handler[ev.type](&ev); /* call handler */
}

void setup(void) {
    screen = DefaultScreen(dpy);
    win = XCreateSimpleWindow(
        dpy, XDefaultRootWindow(dpy),
        1000, 200, width, height, 
        0, 0, 0x121212
    );

    drw = drw_create(dpy, screen, win, width, height);
    if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
        die("no fonts could be loaded.");
    
    lrpad = drw->fonts->h;

    c_hover = XCreateFontCursor(dpy, XC_hand2);

    XSizeHints sh;
    sh.width = sh.max_width = sh.min_width = width;
    sh.height = sh.max_height = sh.min_height = height;
    sh.flags = PSize | PMinSize | PMaxSize;
    XSetWMNormalHints(dpy, win, &sh);

    XMapWindow(dpy, win);
    XSelectInput(
        dpy, win, 
        PointerMotionMask | KeyPressMask | ExposureMask |
        ButtonPressMask | LeaveWindowMask
    );
}

// events
void expose(XEvent *e) {
    drw_setscheme(drw, drw_scm_create(drw, colors, 2));
    draw_tabs();
    draw_grid();
}

void buttonpress(XEvent *e) {
    XButtonPressedEvent *ev = &e->xbutton;
    int x = ev->x, y = ev->y;
    int c, r;
    char cmd[100];

    // mouse right click
    if (ev->button == 1) {
        if (get_block(x, y, &c, &r)) {
            // headers
            if (r == tabs_row) {
                if (tab != c) {
                    tab = c;
                    scroll = 0;
                    draw_tabs();
                    draw_grid();
                }
            } else {
                if (is_emoji(c, r)) {
                    int e = r * grid + c + scroll * grid;
                    sprintf(cmd, "echo -n %s | xclip -selection clibboard", emojis[tab].emojis[e]);
                    system(cmd);
                }
            }
        }
    }

    // scroll up
    else if (ev->button == 4) {
        if (scroll > 0) {
            scroll--;
            draw_grid();
        }
    }

    // scroll down
    else if (ev->button == 5) {
        if ((scroll + 1) * grid + grid * tabs_row < emojis[tab].length + grid) {
            scroll++;
            draw_grid();
        }
    }
}

void keypress(XEvent *e) {
    XKeyEvent *ev = &e->xkey;
    KeySym keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);

    if (keysym == XK_Escape || keysym == XK_q)
        running = 0;
}

void mousemove(XEvent *e) {
    XMotionEvent *ev = &e->xmotion;
    int x = ev->x, y = ev->y;
    int c, r;
    
    if (get_block(x, y, &c, &r) && (r == tabs_row || is_emoji(c, r))) {
        XDefineCursor(dpy, win, c_hover);
        
        if (crnt_emoji_c != c || crnt_emoji_r != r) {
            update_emoji_focus(c, r); // focus on current
        }
    } else {
        XDefineCursor(dpy, win, 0);
        update_emoji_focus(-1, -1);
    }
}

void window_leave(XEvent *e) {
    update_emoji_focus(-1, -1);
}

// draws
void draw_tabs(void) {
    for (int c = 0; c < grid; c++) {
        draw_emoji(c, tabs_row);
    }
}

void draw_grid(void) {
    int c, r;

    for (r = 0; r < tabs_row; r++) {
        for (c = 0; c < grid; c++) {
            if (is_emoji(c, r)) draw_emoji(c, r);
            else XClearArea(dpy, win, c * gap_box, r * gap_box, box, box, 0);
        }
    }
}

void draw_emoji(int c, int r) {
    int e = r * grid + c + scroll * grid;
    int x = gap_box * c;
    int y = gap_box * r;

    if (r == tabs_row) {
        drw_text(drw, x, y, box, box, 6, tabs[c], 0);

        if (c == tab) {
            XSetForeground(dpy, drw->gc, tab_active);
            XFillRectangle(dpy, win, drw->gc, x, y, box, tab_line);
        }
    } else {
        if (!is_emoji(c, r)) return;
        drw_text(drw, x, y, box, box, 6, emojis[tab].emojis[e], 0);
    }
    
    // focus
    if (crnt_emoji_c != c || crnt_emoji_r != r) return;
    
    if (r == tabs_row) {
        drw_rect(drw, x, y + box - tab_line, box, tab_line, 1, 0);
    } else {
        XSetLineAttributes(dpy, drw->gc, focus_line[0], LineSolid, CapButt, JoinMiter);
        drw_rect(drw, 
            x + focus_line[1], y + focus_line[1], 
            box - focus_line[0], box - focus_line[0], 0, 0
        );
    }
}

// utils
int get_block(int x, int y, int *c, int *r) {
    int cb, rb;
    *c = x / gap_box;
    *r = y / gap_box;
    cb = (*c + 1) * box + (*c * gap);
    rb = (*r + 1) * box + (*r * gap);

    if ((y - rb) <= 0 && (x - cb) <= 0)
        return 1;

    return 0;
}

int is_emoji(int c, int r) {
    return (r * grid + c + scroll * grid < emojis[tab].length);
}

void update_emoji_focus(int c, int r) {
    crnt_emoji_c = c;
    crnt_emoji_r = r;
        
    if (last_emoji_c != -1) {
        draw_emoji(last_emoji_c, last_emoji_r);
        last_emoji_c = c;
        last_emoji_r = r;
    }

    if (c != -1) {
        draw_emoji(c, r);
        last_emoji_c = c;
        last_emoji_r = r;
    }
}

int main() {
    if (!(dpy = XOpenDisplay(NULL)))
        die("orpheus: cannot open display");
    
    setup();
    run();

    XCloseDisplay(dpy);

    return 0;
}