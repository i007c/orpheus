#include <X11/Xlib.h>
#include <stdio.h>

int main() {
    XEvent event;
    Display* display = XOpenDisplay(NULL);
    int screen = DefaultScreen(display);
    GC gc = DefaultGC(display, screen);

    int box = 8 * 45;

    Window window = XCreateSimpleWindow(
        display, XDefaultRootWindow(display),
        50, 50, box, box, 0, 0, 0
    );

    XSetWindowAttributes set_attr;
    set_attr.override_redirect = True;
    XChangeWindowAttributes(display, window, CWOverrideRedirect, &set_attr);
    

    XMapWindow(display, window);
    XSelectInput(display, window, KeyPressMask | ButtonPressMask | ExposureMask);

    XSetForeground(display, gc, 0xffffff);
    int res = XFillRectangle(display, window, gc, 0, 0, 45, 45);
    printf("rec res: %d\n", res);

    while (True) {
        XNextEvent(display, &event);
        printf("type: %d | x: %d\n", event.type, event.xbutton.x);
    }

    return 0;
}