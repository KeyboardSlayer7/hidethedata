#ifndef COPY_H
#define COPY_H

#if defined(_WIN32) || defined(_WIN64)
    #include <Windows.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


static void xCopy(const char* data)
{
    Display* display = XOpenDisplay(NULL);
    Window window = 
        XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 128, 128, 0, 0, 0);
    
    char* supported_targets[2] = {"XA_STRING", "UTF8_STRING"};

    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    Atom targets = XInternAtom(display, "TARGETS", False);
    Atom atom = XInternAtom(display, "XA_ATOM", False);

    Atom supported_targets_atoms[2];
    XInternAtoms(display, supported_targets, 2, False, supported_targets_atoms);

    XSetSelectionOwner(display, clipboard, window, CurrentTime);
    
    bool stop = false;

    time_t start = time(NULL);

    while (!stop)
    {
        XEvent event;
        XNextEvent(display, &event);

        if (event.type == SelectionRequest)
        {
            XSelectionRequestEvent sre = event.xselectionrequest;
            
            if (XGetSelectionOwner(display, clipboard) == window && sre.selection == clipboard)
            {
                if (sre.property != None)
                {
                    if (sre.target == targets)
                    {
                        XChangeProperty(display, sre.requestor, sre.property, atom, 32, PropModeReplace, (unsigned char*)supported_targets_atoms, 2);
                    }
                    else if (sre.target == supported_targets_atoms[0] || sre.target == supported_targets_atoms[1])
                    {
                        XChangeProperty(display, sre.requestor, sre.property, sre.target, 8, PropModeReplace, (unsigned char*)data, strlen(data)); 
                    }

                    XSelectionEvent send_event = {
                        .type = SelectionNotify,
                        .serial = sre.serial,
                        .send_event = sre.send_event,
                        .display = sre.display,
                        .requestor = sre.requestor,
                        .selection = sre.selection,
                        .target = sre.target,
                        .property = sre.property,
                        .time = sre.time
                    };

                    XSendEvent(display, sre.requestor, 0, 0, (XEvent*)&send_event);
                }
            }
        }

        time_t now = time(NULL);
        double elapsed = difftime(now, start);

        if (elapsed > 15)
            stop = true;
    }

    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

static void wCopy(const char* data)
{

}

#endif //COPY_H
