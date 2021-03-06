//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

#include "X11Display.h"

#include "../base/Exception.h"
#include "../base/Logger.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include <boost/math/special_functions/fpclassify.hpp>


namespace avg {

X11Display::X11Display()
{
}

X11Display::~X11Display()
{
}
 
float X11Display::queryPPMM()
{
    ::Display * pDisplay = XOpenDisplay(0);
    float ppmm = getScreenResolution().x/float(DisplayWidthMM(pDisplay, 0));
    XCloseDisplay(pDisplay);
    return ppmm;
}

IntPoint X11Display::queryScreenResolution()
{
    IntPoint size;
    // Xinerama query has been removed from here in order to fix #431
    ::Display * pDisplay = XOpenDisplay(0);

    Screen* pScreen = DefaultScreenOfDisplay(pDisplay);
    AVG_ASSERT(pScreen);
    size = IntPoint(pScreen->width, pScreen->height);

    XCloseDisplay(pDisplay);
    return size;
}

float X11Display::queryRefreshRate()
{
    ::Display * pDisplay = XOpenDisplay(0);
    int pixelClock;
    XF86VidModeModeLine modeLine;
    bool bOK = XF86VidModeGetModeLine(pDisplay, DefaultScreen(pDisplay), 
            &pixelClock, &modeLine);
    if (!bOK) {
        AVG_LOG_WARNING(
                "Could not get current refresh rate (XF86VidModeGetModeLine failed).");
        AVG_LOG_WARNING("Defaulting to 60 Hz refresh rate.");
        return 60;
    }
    float hSyncRate = (pixelClock * 1000.0) / modeLine.htotal;
    float refreshRate = hSyncRate / modeLine.vtotal;
    XCloseDisplay(pDisplay);
    if ( refreshRate < 20 || refreshRate > 200 || !(boost::math::isnormal(refreshRate))){
        AVG_LOG_WARNING("Could not get valid refresh rate");
        AVG_LOG_WARNING("Defaulting to 60 Hz refresh rate.");
        return 60;
    }
    return refreshRate;
}

::Display* getX11Display(const SDL_SysWMinfo* pSDLWMInfo)
{
    ::Display* pDisplay;
    if (pSDLWMInfo) {
        // SDL window exists, use it.
        pDisplay = pSDLWMInfo->info.x11.display;
    } else {
        pDisplay = XOpenDisplay(0);
    }
    if (!pDisplay) {
        throw Exception(AVG_ERR_VIDEO_GENERAL, "No X windows display available.");
    }
    return pDisplay;
}

Window createChildWindow(const SDL_SysWMinfo* pSDLWMInfo, XVisualInfo* pVisualInfo,
        const IntPoint& windowSize, const Colormap& colormap)

{
    // Create a child window with the required attributes to render into.
    XSetWindowAttributes swa;
    ::Display* pDisplay = pSDLWMInfo->info.x11.display;
    swa.colormap = colormap;
    swa.background_pixmap = None;
    swa.event_mask = StructureNotifyMask; 
    Window win = XCreateWindow(pDisplay, pSDLWMInfo->info.x11.window, 
            0, 0, windowSize.x, windowSize.y, 0, pVisualInfo->depth, InputOutput, 
            pVisualInfo->visual, CWColormap|CWEventMask, &swa);
    AVG_ASSERT(win);
    XMapWindow(pDisplay, win);
    return win;
}

}
