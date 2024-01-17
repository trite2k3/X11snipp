/*
	X11 Snipping tool.
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <FreeImage.h>
#include <fstream>

//get coords from where to grab image
int DrawSelect(int& rx, int& ry, int& rw, int& rh)
{
    int done = 0;

    XEvent ev;
    Display *disp = XOpenDisplay(NULL);

    if(!disp)
            return EXIT_FAILURE;

    Screen *scr = NULL;
    scr = ScreenOfDisplay(disp, DefaultScreen(disp));

    Window root = 0;
    root = RootWindow(disp, XScreenNumberOfScreen(scr));

    Cursor cursor;
    cursor = XCreateFontCursor(disp, XC_left_ptr);

    /* this XGrab* stuff makes XPending true ? */
    if ((XGrabPointer
        (disp, root, False,
        ButtonMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync,
        GrabModeAsync, root, cursor, CurrentTime) != GrabSuccess))
        printf("couldn't grab pointer:");

    if ((XGrabKeyboard
        (disp, root, False, GrabModeAsync, GrabModeAsync,
        CurrentTime) != GrabSuccess))
        printf("couldn't grab keyboard:");

    while (!done)
    {
        while (!done && XPending(disp))
        {
        XNextEvent(disp, &ev);
        switch (ev.type)
        {
            case ButtonPress:
                rx = ev.xbutton.x;
                ry = ev.xbutton.y;
            break;
            case ButtonRelease:
                done = 1;
            break;
        }
        }
    }
    rw = ev.xbutton.x - rx;
    rh = ev.xbutton.y - ry;
    /* cursor moves backwards */
    if (rw < 0)
    {
        rx += rw;
        rw = 0 - rw;
    }
    if (rh < 0)
    {
        ry += rh;
        rh = 0 - rh;
    }

    XCloseDisplay(disp);

    return EXIT_SUCCESS;
}

void saveImageToFile(XImage *image, const char *filename)
{
    // Assuming XYPixmap has 24 bits per pixel (8 bits for each RGB channel)
    //int bytes_per_line = image->width * 3; // 3 bytes per pixel for RGB

    FIBITMAP *bitmap = FreeImage_Allocate(image->width, image->height, 24); // 24 bits per pixel for RGB
    if (!bitmap)
    {
        fprintf(stderr, "Error: Couldn't allocate FreeImage bitmap\n");
        return;
    }

    // Iterate through each pixel and set the corresponding RGB value
    for (int y = 0; y < image->height; ++y)
    {
        // Reverse the order of rows to address upside-down issue
        int reversedY = image->height - 1 - y;

        for (int x = 0; x < image->width; ++x)
        {
            unsigned long pixelValue = XGetPixel(image, x, reversedY);

            // Extract RGB channels (assuming 24 bits per pixel)
            BYTE red = (pixelValue >> 16) & 0xFF;
            BYTE green = (pixelValue >> 8) & 0xFF;
            BYTE blue = pixelValue & 0xFF;

            RGBQUAD color = {blue, green, red}; // FreeImage uses BGR order
            FreeImage_SetPixelColor(bitmap, x, y, &color);
        }
    }

    // Check if the file already exists
    std::ifstream file(filename);
    if (file.good())
    {
        // Remove the existing file
        if (remove(filename) != 0)
        {
            fprintf(stderr, "Error: Couldn't remove existing file %s\n", filename);
            FreeImage_Unload(bitmap);
            return;
        }
    }

    // Save the image to the file
    if (!FreeImage_Save(FIF_PNG, bitmap, filename, 0))
    {
        fprintf(stderr, "Error: Couldn't save image to file %s\n", filename);
    }
    else
    {
        printf("Image saved successfully to %s\n", filename);
    }

    FreeImage_Unload(bitmap);
}

//void SaveImage
void SaveImage(int& sx, int& sy, int& ew, int& eh)
{
    XImage *image;
    Display *Display = XOpenDisplay( ( char * ) NULL );

            //Get snippet from X11 display coords
            image = XGetImage ( Display, RootWindow ( Display, DefaultScreen ( Display ) ), sx, sy , ew, eh, AllPlanes,XYPixmap );

            //Debug
            saveImageToFile(image, "screenshot.png");

    //Free memory and resources for image
    XDestroyImage(image);
}

//Main program
int main( int argc, char **argv )
{
    int sx, sy , ew, eh;

    DrawSelect(sx, sy , ew, eh);
    SaveImage(sx, sy , ew, eh);

    return 0;
}
