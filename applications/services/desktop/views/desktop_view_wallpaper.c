#include "desktop_view_wallpaper.h"

#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <locale/locale.h>
#include <power/power_service/power.h>
#include <storage/storage.h>
#include <assets_icons.h>

typedef enum {
    DesktopWallpaperStatusOk,
    DesktopWallpaperStatusNoSd,
    DesktopWallpaperStatusSdError,
    DesktopWallpaperStatusNoDatabases,
    DesktopWallpaperStatusBadBattery,
} DesktopWallpaperStatus;

typedef struct {
    DateTime datetime;
    bool format_12;
    LocaleDateFormat date_format;
    DesktopWallpaperStatus status;
} DesktopViewWallpaperModel;

struct DesktopViewWallpaper {
    View* view;
    FuriPubSubSubscription* storage_subscription;
    volatile bool status_dirty;
};

static const char* const desktop_view_wallpaper_status_text[] = {
    [DesktopWallpaperStatusOk] = NULL,
    [DesktopWallpaperStatusNoSd] = "No SD card",
    [DesktopWallpaperStatusSdError] = "SD card error",
    [DesktopWallpaperStatusNoDatabases] = "No databases",
    [DesktopWallpaperStatusBadBattery] = "Battery worn",
};

static void desktop_view_wallpaper_draw(Canvas* canvas, void* model) {
    DesktopViewWallpaperModel* m = model;

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_icon(canvas, 0, 0, &I_Wallpaper_128x64);

    uint8_t hour = m->datetime.hour;
    if(m->format_12) {
        hour = hour % 12;
        if(hour == 0) hour = 12;
    }

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02u:%02u", hour, m->datetime.minute);
    canvas_set_font(canvas, FontBigNumbers);
    canvas_draw_str(canvas, 3, 34, buffer);

    canvas_set_font(canvas, FontSecondary);
    const char* status_text = desktop_view_wallpaper_status_text[m->status];
    if(status_text) {
        canvas_draw_rbox(canvas, 2, 38, canvas_string_width(canvas, status_text) + 5, 11, 2);
        canvas_set_color(canvas, ColorWhite);
        canvas_draw_str(canvas, 5, 46, status_text);
        canvas_set_color(canvas, ColorBlack);
    } else {
        FuriString* date = furi_string_alloc();
        locale_format_date(date, &m->datetime, m->date_format, ".");
        canvas_draw_str(canvas, 4, 46, furi_string_get_cstr(date));
        furi_string_free(date);
    }
}

static bool desktop_view_wallpaper_input(InputEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);
    return false;
}

static void desktop_view_wallpaper_storage_callback(const void* message, void* context) {
    UNUSED(message);
    DesktopViewWallpaper* wallpaper = context;
    // Runs in the storage thread: postpone the actual checks to the desktop thread
    wallpaper->status_dirty = true;
}

static DesktopWallpaperStatus desktop_view_wallpaper_get_status(void) {
    DesktopWallpaperStatus status = DesktopWallpaperStatusOk;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FS_Error sd_status = storage_sd_status(storage);
    if(sd_status == FSE_NOT_READY) {
        status = DesktopWallpaperStatusNoSd;
    } else if(sd_status != FSE_OK) {
        status = DesktopWallpaperStatusSdError;
    } else if(!storage_file_exists(storage, EXT_PATH("Manifest"))) {
        status = DesktopWallpaperStatusNoDatabases;
    }
    furi_record_close(RECORD_STORAGE);

    if(status == DesktopWallpaperStatusOk) {
        Power* power = furi_record_open(RECORD_POWER);
        if(!power_is_battery_healthy(power)) {
            status = DesktopWallpaperStatusBadBattery;
        }
        furi_record_close(RECORD_POWER);
    }

    return status;
}

void desktop_view_wallpaper_update(DesktopViewWallpaper* wallpaper) {
    furi_assert(wallpaper);

    DateTime datetime;
    furi_hal_rtc_get_datetime(&datetime);
    const bool format_12 = locale_get_time_format() == LocaleTimeFormat12h;
    const LocaleDateFormat date_format = locale_get_date_format();

    const bool status_dirty = wallpaper->status_dirty;
    wallpaper->status_dirty = false;
    DesktopWallpaperStatus status = DesktopWallpaperStatusOk;
    if(status_dirty) {
        status = desktop_view_wallpaper_get_status();
    }

    bool changed = false;
    with_view_model(
        wallpaper->view,
        DesktopViewWallpaperModel * model,
        {
            changed = model->datetime.minute != datetime.minute ||
                      model->datetime.hour != datetime.hour ||
                      model->datetime.day != datetime.day || model->format_12 != format_12 ||
                      model->date_format != date_format;
            model->datetime = datetime;
            model->format_12 = format_12;
            model->date_format = date_format;
            if(status_dirty && model->status != status) {
                model->status = status;
                changed = true;
            }
        },
        changed); // Avoid redrawing the screen when nothing visible has changed
}

DesktopViewWallpaper* desktop_view_wallpaper_alloc(void) {
    DesktopViewWallpaper* wallpaper = malloc(sizeof(DesktopViewWallpaper));

    wallpaper->view = view_alloc();
    view_allocate_model(wallpaper->view, ViewModelTypeLocking, sizeof(DesktopViewWallpaperModel));
    view_set_context(wallpaper->view, wallpaper);
    view_set_draw_callback(wallpaper->view, desktop_view_wallpaper_draw);
    view_set_input_callback(wallpaper->view, desktop_view_wallpaper_input);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    wallpaper->storage_subscription = furi_pubsub_subscribe(
        storage_get_pubsub(storage), desktop_view_wallpaper_storage_callback, wallpaper);
    furi_record_close(RECORD_STORAGE);

    wallpaper->status_dirty = true;

    return wallpaper;
}

void desktop_view_wallpaper_free(DesktopViewWallpaper* wallpaper) {
    furi_assert(wallpaper);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_pubsub_unsubscribe(storage_get_pubsub(storage), wallpaper->storage_subscription);
    furi_record_close(RECORD_STORAGE);

    view_free(wallpaper->view);
    free(wallpaper);
}

View* desktop_view_wallpaper_get_view(DesktopViewWallpaper* wallpaper) {
    furi_assert(wallpaper);
    return wallpaper->view;
}
