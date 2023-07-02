
#include "orpheus.h"
#include "drw.h"
#include "emojis.h"


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
static void calc_grid(void);
static void draw_emoji(int c, int r);

// utils
int get_block(int x, int y, int *c, int *r);
int is_emoji(int c, int r);
void update_emoji_focus(int c, int r);
void update_scroll(int change);
void copy_emoji(int c, int r);
void keyboard_movement(short move);
void update_tab(short index);


/* variables */
static void (*handler[LASTEvent])(XEvent *) = {
    [ButtonPress] = buttonpress,
    [Expose] = expose,
    [MotionNotify] = mousemove,
    [KeyPress] = keypress,
    [LeaveNotify] = window_leave,
};
static Bool running = True;
static int screen;
static Display *dpy;
static Window win;
static Drw *drw;
static int lrpad;
static unsigned int scroll = 0;
static Cursor c_hover;
// current emoji pos
static int crnt_emoji_c = 0;
static int crnt_emoji_r = 0;
// last emoji pos
static int last_emoji_c = 0;
static int last_emoji_r = 0;

#include "config.h"

typedef struct EmojiGrid {
    FT_UInt id;
    Emoji *e;
    int32_t child;
} EmojiGrid;

static EmojiGrid grid[GRID_BOX][GRID_BOX];


void run(void) {
    XEvent ev;
    /* main event loop */
    XSync(dpy, False);
    while (running && !XNextEvent(dpy, &ev))
        if (handler[ev.type])
            handler[ev.type](&ev); /* call handler */
}

void setup(void) {
    XSizeHints sh;

    memset(grid, 0, sizeof(grid));

    // calculate the max scroll for each emoji_set
    int32_t i, ms;
    uint16_t group = 0;

    for (i = 0; i < GRID_BOX; i++) {
        ms = (emojis[i].length + GRID_BOX - 1 - GRID_BOX * tabs_row) / GRID_BOX;
        emojis[i].max_scroll = ms < 0 ? 0 : ms;

        for (size_t j = 0; j < emojis[i].length; j++) {
            group = emojis[i].set[j].group;
            if (group)
                emojis[i].set[j].id = EmojiGroups[group][EG_COLOR];
        }
    }
    
    screen = DefaultScreen(dpy);
    win = XCreateSimpleWindow(
        dpy, XDefaultRootWindow(dpy),
        1000, 200, width, height, 
        0, 0, 0x121212
    );

    drw = drw_create(dpy, screen, win, width, height);
    if (!drw_fontset_create(drw, fonts, LENGTH(fonts)))
        panic("no fonts could be loaded.");
    
    lrpad = drw->fonts->h;

    c_hover = XCreateFontCursor(dpy, XC_hand2);

    sh.min_width  = sh.max_width  = width;
    sh.min_height = sh.max_height = height;
    sh.flags = PMinSize | PMaxSize;
    XSetWMNormalHints(dpy, win, &sh);

    XStoreName(dpy, win, "Orpheus");

    XMapWindow(dpy, win);
    XSelectInput(
        dpy, win, 
        PointerMotionMask | KeyPressMask | ExposureMask |
        ButtonPressMask | LeaveWindowMask
    );
}

// events
void expose(XEvent *UNUSED(E)) {
    drw_setscheme(drw, drw_scm_create(drw, colors, 2));
    draw_tabs();
    calc_grid();
    // draw_grid();
}

void buttonpress(XEvent *e) {
    XButtonPressedEvent *ev = &e->xbutton;
    int x = ev->x, y = ev->y;
    int c, r;
    EmojiGrid emoji;

    if (ev->button == 1) {
        // mouse right click
        if (get_block(x, y, &c, &r)) {
            if (r == tabs_row)
                update_tab(c);
            else
                copy_emoji(c, r);

        }
    } else if (ev->button == 3) {
        // mouse left click
        if (get_block(x, y, &c, &r)) {
            if (r == tabs_row) return;
            emoji = grid[r][c];
            if (!emoji.e->group) return;
            emoji.e->expand = !emoji.e->expand;
            calc_grid();
            // draw_grid();
        }
    } else if (ev->button == 4) {
        // scroll up
        update_scroll(-1);
    } else if (ev->button == 5) {
        // scroll down
        update_scroll(1);
    }
}

void keypress(XEvent *e) {
    XKeyEvent *ev = &e->xkey;
    KeySym keysym = XkbKeycodeToKeysym(dpy, ev->keycode, 0, ev->state & ShiftMask ? 1 : 0);
    // KeySym keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);

    switch (keysym) {
        case XK_Escape:
        case XK_q:
            running = False;
            return;
        
        case XK_Page_Up:
            update_scroll(-1);
            break;

        case XK_Page_Down:
            update_scroll(1);
            break;
        
        case XK_Return:
            if (crnt_emoji_c != -1) {
                if (crnt_emoji_r == tabs_row)
                    update_tab(crnt_emoji_c);
                else
                    copy_emoji(crnt_emoji_c, crnt_emoji_r);
            }
            break;
        
        case XK_Up:
            keyboard_movement(0);
            break;

        case XK_Right:
            if (ev->state == ShiftMask) update_tab(tab + 1);
            else keyboard_movement(1);
            break;

        case XK_Down:
            keyboard_movement(2);
            break;
            
        case XK_Left:
            if (ev->state == ShiftMask) update_tab(tab - 1);
            else keyboard_movement(3);
            break;
        
        case XK_Home:
            scroll = 0;
            calc_grid();
            // draw_grid();
            update_emoji_focus(0, 0);
            break;

        case XK_End:
            if (emojis[tab].max_scroll < 1 || 
                scroll == emojis[tab].max_scroll) break;

            scroll = emojis[tab].max_scroll;
            calc_grid();
            // draw_grid();
            update_emoji_focus(GRID_BOX - 1, tabs_row - 2);
            break;
        
        case XK_Tab:
            if (crnt_emoji_c == -1) crnt_emoji_c = 0;
            if (crnt_emoji_r != tabs_row) update_emoji_focus(crnt_emoji_c, tabs_row);
            else update_emoji_focus(0, 0);
    }
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

void window_leave(XEvent *UNUSED(E)) {
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

void update_scroll(int change) {
    int32_t s = scroll + change;
    if (s < 0) s = 0;
    if ((uint32_t)s > emojis[tab].max_scroll) s = emojis[tab].max_scroll;
    if ((uint32_t)s == scroll) return;

    scroll = s;
    draw_grid();
}

void copy_emoji(int c, int r) {
    int e;
    char cmd[100];

    if (is_emoji(c, r)) {
        e = r * grid + c + scroll * grid;
        sprintf(cmd, "echo -n %s | xclip -sel p -f | xclip -sel c", emojis[tab].emojis[e]);
        system(cmd);
    }
}

void keyboard_movement(short move) {
    int c = crnt_emoji_c;
    int r = crnt_emoji_r;

    if (c == -1) {
        update_emoji_focus(0, 0);
        return;
    }

    /*
     * moves:
     * 0 = up
     * 1 = right
     * 2 = down
     * 3 = left
     */
    switch (move) {
        case 0:
            if (r > 0) {
                if (is_emoji(c, r - 1)) update_emoji_focus(c, r - 1);
            } else {
                update_scroll(-1);
            }
                
            return;

        case 1:
            if (c < grid - 1) {
                if (r == tabs_row || is_emoji(c + 1, r)) update_emoji_focus(c + 1, r);
            } else {
                if (is_emoji(0, r + 1)) update_emoji_focus(0, r + 1);
                else update_emoji_focus(0, r);
            }
            return;

        case 2:
            if (r < tabs_row - 1) {
                if (is_emoji(c, r + 1)) update_emoji_focus(c, r + 1);
            } else {
                update_scroll(1);
                // update_emoji_focus(c, r - 1);
            }
            return;

        case 3:
            if (c > 0) {
                if (r == tabs_row || is_emoji(c - 1, r)) update_emoji_focus(c - 1, r);
            } else {
                if (r == tabs_row) update_emoji_focus(grid - 1, r);
                else if (r > 0 && is_emoji(grid - 1, r - 1))
                    update_emoji_focus(grid - 1, r - 1);
                else if (scroll > 0 && r == 0) {
                    update_scroll(-1);
                    update_emoji_focus(grid - 1, 0);
                }
            }
                
            return;
    }
}

void update_tab(short index) {
    if (tab == index) return;

    if (index < 0) index = grid - 1;
    if (index > grid - 1) index = 0;

    tab = index;
    scroll = 0;
    draw_tabs();
    draw_grid();
}

int main(int argc, char *argv[]) {
    if (argc == 2 && !strcmp("-v", argv[1]))
    	panic("orpheus-" VERSION);

    if (!(dpy = XOpenDisplay(NULL)))
        panic("orpheus: cannot open display");
    
    setup();
    run();

    XCloseDisplay(dpy);

    return 0;
}
