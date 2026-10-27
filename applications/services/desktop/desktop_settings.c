#include "desktop_settings.h"
#include "desktop_settings_filename.h"

#include <saved_struct.h>
#include <storage/storage.h>

#define TAG "DesktopSettings"

#define DESKTOP_SETTINGS_VER_10 (10)
#define DESKTOP_SETTINGS_VER_11 (11)
#define DESKTOP_SETTINGS_VER    (12)

#define DESKTOP_SETTINGS_PATH  INT_PATH(DESKTOP_SETTINGS_FILE_NAME)
#define DESKTOP_SETTINGS_MAGIC (0x17)

// Layout of DesktopSettings v11, before dummy mode was removed
typedef struct {
    uint32_t auto_lock_delay_ms;
    uint8_t dummy_mode;
    uint8_t display_clock;
    FavoriteApp favorite_apps[FavoriteAppNumber];
    FavoriteApp dummy_apps[4];
} DesktopSettingsV11;

// Actual size of DesktopSettings v11
static_assert(sizeof(DesktopSettingsV11) == 1032);

typedef struct {
    uint8_t reserved[11];
    DesktopSettingsV11 settings;
} DesktopSettingsV10;

// Actual size of DesktopSettings v10
static_assert(sizeof(DesktopSettingsV10) == 1044);

static void
    desktop_settings_migrate_v11(DesktopSettings* settings, const DesktopSettingsV11* old) {
    settings->auto_lock_delay_ms = old->auto_lock_delay_ms;
    settings->display_clock = old->display_clock;
    memcpy(settings->favorite_apps, old->favorite_apps, sizeof(settings->favorite_apps));
}

void desktop_settings_load(DesktopSettings* settings) {
    furi_assert(settings);

    bool success = false;

    do {
        uint8_t version;
        if(!saved_struct_get_metadata(DESKTOP_SETTINGS_PATH, NULL, &version, NULL)) break;

        if(version == DESKTOP_SETTINGS_VER) {
            success = saved_struct_load(
                DESKTOP_SETTINGS_PATH,
                settings,
                sizeof(DesktopSettings),
                DESKTOP_SETTINGS_MAGIC,
                DESKTOP_SETTINGS_VER);

        } else if(version == DESKTOP_SETTINGS_VER_11) {
            DesktopSettingsV11* settings_v11 = malloc(sizeof(DesktopSettingsV11));

            success = saved_struct_load(
                DESKTOP_SETTINGS_PATH,
                settings_v11,
                sizeof(DesktopSettingsV11),
                DESKTOP_SETTINGS_MAGIC,
                DESKTOP_SETTINGS_VER_11);

            if(success) {
                desktop_settings_migrate_v11(settings, settings_v11);
            }

            free(settings_v11);

        } else if(version == DESKTOP_SETTINGS_VER_10) {
            DesktopSettingsV10* settings_v10 = malloc(sizeof(DesktopSettingsV10));

            success = saved_struct_load(
                DESKTOP_SETTINGS_PATH,
                settings_v10,
                sizeof(DesktopSettingsV10),
                DESKTOP_SETTINGS_MAGIC,
                DESKTOP_SETTINGS_VER_10);

            if(success) {
                desktop_settings_migrate_v11(settings, &settings_v10->settings);
            }

            free(settings_v10);
        }

    } while(false);

    if(!success) {
        FURI_LOG_W(TAG, "Failed to load file, using defaults");
        memset(settings, 0, sizeof(DesktopSettings));
        desktop_settings_save(settings);
    }
}

void desktop_settings_save(const DesktopSettings* settings) {
    furi_assert(settings);

    const bool success = saved_struct_save(
        DESKTOP_SETTINGS_PATH,
        settings,
        sizeof(DesktopSettings),
        DESKTOP_SETTINGS_MAGIC,
        DESKTOP_SETTINGS_VER);

    if(!success) {
        FURI_LOG_E(TAG, "Failed to save file");
    }
}
