#pragma once

#include <gui/view.h>

typedef struct DesktopViewWallpaper DesktopViewWallpaper;

DesktopViewWallpaper* desktop_view_wallpaper_alloc(void);

void desktop_view_wallpaper_free(DesktopViewWallpaper* wallpaper);

View* desktop_view_wallpaper_get_view(DesktopViewWallpaper* wallpaper);

/** Refresh clock and status hints. Must be called from the desktop thread. */
void desktop_view_wallpaper_update(DesktopViewWallpaper* wallpaper);
