
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
static void calc_max_scroll(int idx);

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
static XftDraw *xftdraw;
static int lrpad;
static unsigned int scroll = 0;
static Cursor c_hover;
static size_t expand_length = 0;
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

    expand_length = 0;
    memset(grid, 0, sizeof(grid));

    // calculate the max scroll for each emoji_set
    int32_t i;
    uint16_t group = 0;

    for (i = 0; i < GRID_BOX; i++) {
        calc_max_scroll(i);

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

    xftdraw = XftDrawCreate(
        drw->dpy, drw->drawable,
        DefaultVisual(drw->dpy, drw->screen),
        DefaultColormap(drw->dpy, drw->screen)
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
            if (!emoji.e) return;
            printf("id: %d - g: %d\n", emoji.id, emoji.e->group);
            if (!emoji.e || !emoji.e->group) return;

            emoji.e->expand = !emoji.e->expand;

            if (emoji.e->expand) {
                expand_length += 5;
            } else {
                expand_length -= 5;
            }

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
    for (int c = 0; c < GRID_BOX; c++) {
        draw_emoji(c, tabs_row);
    }
}

void calc_max_scroll(int idx) {
    size_t ms = emojis[idx].length + expand_length + GRID_BOX - 1;
    if (ms < GRID_BOX * (GRID_BOX - 1)) {
        emojis[idx].max_scroll = 0;
        return;
    }
    ms -= GRID_BOX * (GRID_BOX - 1);
    emojis[idx].max_scroll = ms / GRID_BOX;
}

void calc_grid(void) {
    uint8_t c = 0, r = 0, g;
    size_t e = GRID_BOX * scroll;
    size_t idx = 0;
    EmojiGrid *eg;
    uint32_t id;

    for (idx = 0; idx < GRID_BOX * scroll; idx++) {
        if (emojis[tab].set[idx].expand) expand_length -= 5;
        emojis[tab].set[idx].expand = false;
    }

    for (idx = e + GRID_BOX * (GRID_BOX-1); idx < emojis[tab].length; idx++) {
        if (emojis[tab].set[idx].expand) expand_length -= 5;
        emojis[tab].set[idx].expand = false;
    }

    calc_max_scroll(tab);

    for (r = 0; r < tabs_row; r++) {
        for (c = 0; c < GRID_BOX; c++) {
            eg = &grid[r][c];

            if (e >= emojis[tab].length) {
                eg->e = NULL;
                continue;
            }

            eg->e = &emojis[tab].set[e];
            eg->id = eg->e->id;
            eg->child = -1;

            e++;

            if (eg->e->expand && eg->e->group) {
                for (g = 0; g < 6; g++) {
                    id = EmojiGroups[eg->e->group][g];
                    if (id == eg->e->id) continue;

                    c++;
                    if (c > GRID_BOX-1) {
                        r++;
                        c = 0;
                    }

                    grid[r][c].e = eg->e;
                    grid[r][c].id = id;
                    grid[r][c].child = 6 - g;
                }
            } 
        }
    }

    draw_grid();
}


void draw_grid(void) {
    uint8_t r, c;
    EmojiGrid *eg;
    int x, y, ty;

    XPoint points[4];

    for (r = 0; r < tabs_row; r++) {
        y = gap_box * r;
        ty = y + (EMOT_BOX - drw->fonts->h) / 2 + drw->fonts->xfont->ascent;
        for (c = 0; c < GRID_BOX; c++) {
            x = gap_box * c;
            eg = &grid[r][c];

            if (eg->e == NULL) {
                XClearArea(dpy, win, x, y, EMOT_BOX, EMOT_BOX, 0);
                continue;
            }

            XSetForeground(drw->dpy, drw->gc, drw->scheme[ColBg].pixel);
            XFillRectangle(
                drw->dpy, drw->drawable, drw->gc,
                x, y, EMOT_BOX, EMOT_BOX
            );
            XftDrawGlyphs(
                xftdraw, &drw->scheme[ColFg],
                drw->fonts->xfont, x+6, ty, &eg->id, 1
            );

            if (eg->child > 0) {
                XSetLineAttributes(
                    dpy, drw->gc, FLW,
                    LineSolid, CapButt, JoinMiter
                );
                XSetForeground(
                    drw->dpy, drw->gc, tab_active
                );

                if (eg->child == 1) {
                    points[0].x = x + FLW2;
                    points[0].y = y + FLW2;

                    points[1].x = EMOT_BOX - FLW;
                    points[1].y = 0;

                    points[2].x = 0;
                    points[2].y = EMOT_BOX - FLW;

                    points[3].x = -EMOT_BOX + FLW2;
                    points[3].y = 0;

                    XDrawLines(
                        drw->dpy, drw->drawable, drw->gc,
                        points, 4, CoordModePrevious
                    );
                 } else {
                    points[0].x = x + EMOT_BOX - FLW2;
                    points[0].y = y + FLW2;

                    points[1].x = -EMOT_BOX + FLW;
                    points[1].y = 0;

                    XDrawLines(
                        drw->dpy, drw->drawable, drw->gc,
                        points, 2, CoordModePrevious
                    );

                    points[0].y = y + FLW2 + (EMOT_BOX - FLW);

                    XDrawLines(
                        drw->dpy, drw->drawable, drw->gc,
                        points, 2, CoordModePrevious
                    );
                }
            }

            if (eg->e->expand && eg->child == -1) {
                points[0].x = x + EMOT_BOX - FLW2;
                points[0].y = y + FLW2;

                points[1].x = -EMOT_BOX + FLW2 * 2;
                points[1].y = 0;

                points[2].x = 0;
                points[2].y = EMOT_BOX - FLW2 * 2;

                points[3].x = EMOT_BOX - FLW2;
                points[3].y = 0;

                XSetLineAttributes(
                    dpy, drw->gc, FLW,
                    LineSolid, CapButt, JoinMiter
                );

                XSetForeground(
                    drw->dpy, drw->gc, tab_active
                );

                XDrawLines(
                    drw->dpy, drw->drawable, drw->gc,
                    points, 4, CoordModePrevious
                );
            }
        }
    }
}

void draw_emoji(int c, int r) {
    int x = gap_box * c;
    int y = gap_box * r;
    EmojiGrid *eg = NULL;
    int ty = y + (EMOT_BOX - drw->fonts->h) / 2 + drw->fonts->xfont->ascent;
    XPoint points[4];

    XSetForeground(drw->dpy, drw->gc, drw->scheme[ColBg].pixel);
    XFillRectangle(drw->dpy, drw->drawable, drw->gc, x, y, EMOT_BOX, EMOT_BOX);

    // XClearArea(dpy, win, x, y, box, box, 0);

    if (r == tabs_row) {
        XftDrawGlyphs(
            xftdraw, &drw->scheme[ColFg],
            drw->fonts->xfont, x+6, ty, &tabs[c], 1
        );
        // drw_text(drw, x, y, box, box, 6, tabs[c], 0);

        if (c == tab) {
            XSetForeground(dpy, drw->gc, tab_active);
            XFillRectangle(dpy, win, drw->gc, x, y, EMOT_BOX, tab_line);
        }
    } else {
        if (!is_emoji(c, r)) return;
        eg = &grid[r][c];
        // drw_text(drw, x, y, box, box, 6, emojis[tab].emojis[e], 0);
        XftDrawGlyphs(
            xftdraw, &drw->scheme[ColFg],
            drw->fonts->xfont, x+6, ty, &eg->id, 1
        );
    }
    
    // focus
    if (crnt_emoji_c != c || crnt_emoji_r != r) {
        if (r == tabs_row) return;

        if (eg->child > 0) {
            XSetLineAttributes(
                dpy, drw->gc, FLW,
                LineSolid, CapButt, JoinMiter
            );
            XSetForeground(
                drw->dpy, drw->gc, tab_active
            );

            if (eg->child == 1) {
                points[0].x = x + FLW2;
                points[0].y = y + FLW2;

                points[1].x = EMOT_BOX - FLW;
                points[1].y = 0;

                points[2].x = 0;
                points[2].y = EMOT_BOX - FLW;

                points[3].x = -EMOT_BOX + FLW2;
                points[3].y = 0;

                XDrawLines(
                    drw->dpy, drw->drawable, drw->gc,
                    points, 4, CoordModePrevious
                );
             } else {
                points[0].x = x + EMOT_BOX - FLW2;
                points[0].y = y + FLW2;

                points[1].x = -EMOT_BOX + FLW;
                points[1].y = 0;

                XDrawLines(
                    drw->dpy, drw->drawable, drw->gc,
                    points, 2, CoordModePrevious
                );

                points[0].y = y + FLW2 + (EMOT_BOX - FLW);

                XDrawLines(
                    drw->dpy, drw->drawable, drw->gc,
                    points, 2, CoordModePrevious
                );
            }
        }

        if (eg->e->expand && eg->child == -1) {
            points[0].x = x + EMOT_BOX - FLW2;
            points[0].y = y + FLW2;

            points[1].x = -EMOT_BOX + FLW2 * 2;
            points[1].y = 0;

            points[2].x = 0;
            points[2].y = EMOT_BOX - FLW2 * 2;

            points[3].x = EMOT_BOX - FLW2;
            points[3].y = 0;

            XSetLineAttributes(
                dpy, drw->gc, FLW,
                LineSolid, CapButt, JoinMiter
            );

            XSetForeground(
                drw->dpy, drw->gc, tab_active
            );

            XDrawLines(
                drw->dpy, drw->drawable, drw->gc,
                points, 4, CoordModePrevious
            );
        }

        return;
    }
    
    if (r == tabs_row) {
        drw_rect(drw, x, y + EMOT_BOX - tab_line, EMOT_BOX, tab_line, 1, 0);
    } else {
        XSetLineAttributes(dpy, drw->gc, FLW, LineSolid, CapButt, JoinMiter);
        drw_rect(drw, 
            x + FLW2, y + FLW2, 
            EMOT_BOX - FLW, EMOT_BOX - FLW, 0, 0
        );
    }
}

// utils
int get_block(int x, int y, int *c, int *r) {
    int cb, rb;
    *c = x / gap_box;
    *r = y / gap_box;
    cb = (*c + 1) * EMOT_BOX + (*c * gap);
    rb = (*r + 1) * EMOT_BOX + (*r * gap);

    if ((y - rb) <= 0 && (x - cb) <= 0)
        return 1;

    return 0;
}

int is_emoji(int c, int r) {
    return (grid[r][c].e != NULL);
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
    calc_grid();
}

void copy_emoji(int c, int r) {
    char cmd[100];

    if (is_emoji(c, r)) {
        sprintf(
            cmd,
            "echo -n %s | xclip -sel p -f | xclip -sel c",
            gidmap[grid[r][c].id]
        );
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
            if (c < GRID_BOX - 1) {
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
                if (r == tabs_row) update_emoji_focus(GRID_BOX - 1, r);
                else if (r > 0 && is_emoji(GRID_BOX - 1, r - 1))
                    update_emoji_focus(GRID_BOX - 1, r - 1);
                else if (scroll > 0 && r == 0) {
                    update_scroll(-1);
                    update_emoji_focus(GRID_BOX - 1, 0);
                }
            }
                
            return;
    }
}

void update_tab(short index) {
    if (tab == index) return;

    if (index < 0) index = GRID_BOX - 1;
    if (index > GRID_BOX - 1) index = 0;

    for (size_t i = 0; i < emojis[tab].length; i++) {
        emojis[tab].set[i].expand = false;
    }

    tab = index;
    expand_length = 0;
    scroll = 0;
    draw_tabs();
    calc_grid();
}

int main(int argc, char *argv[]) {
    if (argc == 2 && !strcmp("-v", argv[1]))
    	panic("orpheus-" VERSION);

    if (!(dpy = XOpenDisplay(NULL)))
        panic("orpheus: cannot open display");
    
    setup();
    run();

    XftDrawDestroy(xftdraw);
    drw_free(drw);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}
