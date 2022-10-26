#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <stdio.h>

#include "drw.h"
#include "util.h"

#include "config.h"

#define LENGTH(X) (sizeof X / sizeof X[0])
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + lrpad)

static void run(void);
static void setup(void);

// events
static void expose(XEvent *e);
static void buttonpress(XEvent *e);
static void keypress(XEvent *e);
static void mousemove(XEvent *e);

// draws
static void draw_tabs(void);
static void draw_grid(void);
static void draw_emoji(int r, int c, int focus);

// utils
int get_block(int x, int y, int *r, int *c);
int is_emoji(int r, int c);


/* variables */
static void (*handler[LASTEvent])(XEvent *) = {
    [ButtonPress] = buttonpress,
    [Expose] = expose,
    [MotionNotify] = mousemove,
    [KeyPress] = keypress,
};
static int running = 1;
static int screen;
static Display *dpy;
static Window win;
static Drw *drw;
static const int gap = 2;
static const int grid = 8;
static const int tabs_row = grid - 1;
static const int box = 45;
static const int gap_box = box + gap;
static const int width = (grid * box) + ((grid - 1) * gap);
static const int height = width;
static const char *colors[] = {"#f2f2f2", "#040404"};
static const int focus_line[2] = {4, 2};
static int lrpad;
static int tab = 0;
// static int focus_box[2] = {-1, -1};
static int scroll = 0;
static int last_emoji_x = -1;
static int last_emoji_y = -1;
static Cursor c_hover;


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
        ButtonPressMask
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
    int r, c;

    // mouse right click
    if (ev->button == 1) {
        if (get_block(x, y, &r, &c)) {
            printf("row: %d | col: %d\n", r, c);
            // headers
            if (r == tabs_row) {
                if (tab != c) {
                    tab = c;
                    draw_tabs();
                    draw_grid();
                }
            } else {
                
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
    int r, c;
    
    if (get_block(x, y, &r, &c) && (r == tabs_row || is_emoji(r, c))) {
        XDefineCursor(dpy, win, c_hover);
        
        if (last_emoji_x != r || last_emoji_y != c) {
            if (last_emoji_x != -1) draw_emoji(last_emoji_x, last_emoji_y, 0);
            draw_emoji(r, c, 1);
            last_emoji_x = r;
            last_emoji_y = c;
        }
    } else {
        XDefineCursor(dpy, win, 0);
        if (last_emoji_x != -1) {
            draw_emoji(last_emoji_x, last_emoji_y, 0);
            last_emoji_x = -1;
            last_emoji_y = -1;
        }
    }
}

// draws
void draw_tabs(void) {
    for (int c = 0; c < grid; c++) {
        draw_emoji(7, c, 0);
    }
}

void draw_grid(void) {
    int x, y, r, c, e;

    // XClearArea(dpy, win, 0, 0, width, height - box, 0);
    
    for (r = 0; r < tabs_row; r++) {
        y = gap_box * r;
        for (c = 0; c < grid; c++) {
            if (!is_emoji(r, c)) return;
            x = gap_box * c;
            draw_emoji(r, c, 0);
        }
    }
}

void draw_emoji(int r, int c, int focus) {
    int e = r * grid + c + scroll * grid;
    int x = gap_box * c;
    int y = gap_box * r;

    if (r == tabs_row)
        drw_text(drw, x, y, box, box, 6, tabs[c], 0);
    else 
        drw_text(drw, x, y, box, box, 6, emojis[tab].emojis[e], 0);
    
    if (!focus) return;
    

    if (r == tabs_row) {
        drw_rect(drw, x, y + box - 4, box, 4, 1, 0);
    } else {
        XSetLineAttributes(dpy, drw->gc, focus_line[0], LineSolid, CapButt, JoinMiter);
        drw_rect(drw, 
            x + focus_line[1], y + focus_line[1], 
            box - focus_line[0], box - focus_line[0], 0, 0
        );
    }
}

// utils
int get_block(int x, int y, int *r, int *c) {
    int rb, cb;
    *r = y / gap_box;
    *c = x / gap_box;
    rb = (*r + 1) * box + (*r * gap);
    cb = (*c + 1) * box + (*c * gap);

    if ((y - rb) <= 0 && (x - cb) <= 0)
        return 1;

    return 0;
}

int is_emoji(int r, int c) {
    return (r * grid + c + scroll * grid < emojis[tab].length);
}

int main() {
    if (!(dpy = XOpenDisplay(NULL)))
        die("orpheus: cannot open display");
    
    setup();
    run();

    return 0;
}