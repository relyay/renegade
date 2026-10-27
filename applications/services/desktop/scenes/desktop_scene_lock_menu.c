#include <gui/scene_manager.h>
#include <applications.h>
#include <furi_hal.h>
#include <toolbox/saved_struct.h>
#include <stdbool.h>
#include <loader/loader.h>
#ifdef SRV_BT
#include <bt/bt_service/bt.h>
#include <bt/bt_service/bt_settings_api_i.h>
#endif

#include "../desktop_i.h"
#include <desktop/desktop_settings.h>
#include "../views/desktop_view_lock_menu.h"
#include "desktop_scene.h"

#define TAG "DesktopSceneLock"

static void desktop_scene_lock_menu_update_bt_state(Desktop* desktop) {
    bool available = false;
    bool enabled = false;
#ifdef SRV_BT
    available = furi_hal_bt_is_gatt_gap_supported();
    if(available) {
        Bt* bt = furi_record_open(RECORD_BT);
        BtSettings settings;
        bt_get_settings(bt, &settings);
        enabled = settings.enabled;
        furi_record_close(RECORD_BT);
    }
#endif
    desktop_lock_menu_set_bt_state(desktop->lock_menu, available, enabled);
}

// Brightness steps the control center cycles through, in percent
static const uint8_t desktop_scene_lock_menu_brightness_levels[] = {10, 25, 50, 75, 100};

static uint8_t desktop_scene_lock_menu_get_brightness(Desktop* desktop) {
    return (uint8_t)(desktop->notification->settings.display_brightness * 100.0f + 0.5f);
}

static void desktop_scene_lock_menu_next_brightness(Desktop* desktop) {
    const uint8_t current = desktop_scene_lock_menu_get_brightness(desktop);
    uint8_t next = desktop_scene_lock_menu_brightness_levels[0];
    for(size_t i = 0; i < COUNT_OF(desktop_scene_lock_menu_brightness_levels); i++) {
        if(desktop_scene_lock_menu_brightness_levels[i] > current) {
            next = desktop_scene_lock_menu_brightness_levels[i];
            break;
        }
    }

    desktop->notification->settings.display_brightness = next / 100.0f;
    notification_message(desktop->notification, &sequence_display_backlight_on);
    notification_message_save_settings(desktop->notification);
    desktop_lock_menu_set_brightness(desktop->lock_menu, next);
}

static void desktop_scene_lock_menu_set_bt_enabled(bool enabled) {
#ifdef SRV_BT
    Bt* bt = furi_record_open(RECORD_BT);
    BtSettings settings;
    bt_get_settings(bt, &settings);
    settings.enabled = enabled;
    bt_set_settings(bt, &settings);
    furi_record_close(RECORD_BT);
#else
    UNUSED(enabled);
#endif
}

void desktop_scene_lock_menu_callback(DesktopEvent event, void* context) {
    Desktop* desktop = (Desktop*)context;
    view_dispatcher_send_custom_event(desktop->view_dispatcher, event);
}

void desktop_scene_lock_menu_on_enter(void* context) {
    Desktop* desktop = (Desktop*)context;

    scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
    desktop_lock_menu_set_callback(desktop->lock_menu, desktop_scene_lock_menu_callback, desktop);
    desktop_lock_menu_set_stealth_mode_state(
        desktop->lock_menu, furi_hal_rtc_is_flag_set(FuriHalRtcFlagStealthMode));
    desktop_scene_lock_menu_update_bt_state(desktop);
    desktop_lock_menu_set_clock_state(desktop->lock_menu, desktop->settings.display_clock);
    desktop_lock_menu_set_brightness(
        desktop->lock_menu, desktop_scene_lock_menu_get_brightness(desktop));
    desktop_lock_menu_set_idx(desktop->lock_menu, 0);

    view_dispatcher_switch_to_view(desktop->view_dispatcher, DesktopViewIdLockMenu);
}

bool desktop_scene_lock_menu_on_event(void* context, SceneManagerEvent event) {
    Desktop* desktop = (Desktop*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        bool check_pin_changed =
            scene_manager_get_scene_state(desktop->scene_manager, DesktopSceneLockMenu);
        if(check_pin_changed && desktop_pin_code_is_set()) {
            scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
        }
    } else if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopLockMenuEventLock:
            scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
            desktop_lock(desktop);
            consumed = true;
            break;
        case DesktopLockMenuEventStealthModeOn:
        case DesktopLockMenuEventStealthModeOff:
            desktop_set_stealth_mode_state(
                desktop, event.event == DesktopLockMenuEventStealthModeOn);
            desktop_lock_menu_set_stealth_mode_state(
                desktop->lock_menu, event.event == DesktopLockMenuEventStealthModeOn);
            consumed = true;
            break;
        case DesktopLockMenuEventBtOn:
        case DesktopLockMenuEventBtOff:
            desktop_scene_lock_menu_set_bt_enabled(event.event == DesktopLockMenuEventBtOn);
            desktop_scene_lock_menu_update_bt_state(desktop);
            consumed = true;
            break;
        case DesktopLockMenuEventBrightnessNext:
            desktop_scene_lock_menu_next_brightness(desktop);
            consumed = true;
            break;
        case DesktopLockMenuEventClockOn:
        case DesktopLockMenuEventClockOff:
            desktop->settings.display_clock = event.event == DesktopLockMenuEventClockOn;
            desktop_lock_menu_set_clock_state(desktop->lock_menu, desktop->settings.display_clock);
            view_dispatcher_send_custom_event(desktop->view_dispatcher, DesktopGlobalSaveSettings);
            consumed = true;
            break;
        case DesktopLockMenuEventPowerOff:
            scene_manager_search_and_switch_to_previous_scene(
                desktop->scene_manager, DesktopSceneMain);
            loader_start_detached_with_gui_error(desktop->loader, "Power", "off");
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(desktop->scene_manager, DesktopSceneLockMenu, 0);
    }
    return consumed;
}

void desktop_scene_lock_menu_on_exit(void* context) {
    UNUSED(context);
}
