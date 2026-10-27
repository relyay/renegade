#include <furi.h>
#include <gui/elements.h>
#include <assets_icons.h>

#include "../desktop_i.h"
#include "desktop_view_lock_menu.h"

// Control center: a 3x2 grid of tiles below the status bar
#define TILE_COLUMNS   3
#define TILE_W         40
#define TILE_H         24
#define TILE_GAP_X     3
#define TILE_GAP_Y     2
#define TILE_X0        1
#define TILE_Y0        (STATUS_BAR_Y_SHIFT + 1)
#define TILE_ICON_SIZE 13

typedef enum {
    DesktopLockMenuIndexLock,
    DesktopLockMenuIndexSound,
    DesktopLockMenuIndexBluetooth,
    DesktopLockMenuIndexBrightness,
    DesktopLockMenuIndexClock,
    DesktopLockMenuIndexPower,

    DesktopLockMenuIndexTotalCount
} DesktopLockMenuIndex;

void desktop_lock_menu_set_callback(
    DesktopLockMenuView* lock_menu,
    DesktopLockMenuViewCallback callback,
    void* context) {
    furi_assert(lock_menu);
    furi_assert(callback);
    lock_menu->callback = callback;
    lock_menu->context = context;
}

void desktop_lock_menu_set_stealth_mode_state(DesktopLockMenuView* lock_menu, bool stealth_mode) {
    with_view_model(
        lock_menu->view,
        DesktopLockMenuViewModel * model,
        { model->stealth_mode = stealth_mode; },
        true);
}

void desktop_lock_menu_set_bt_state(DesktopLockMenuView* lock_menu, bool available, bool enabled) {
    with_view_model(
        lock_menu->view,
        DesktopLockMenuViewModel * model,
        {
            model->bt_available = available;
            model->bt_enabled = enabled;
        },
        true);
}

void desktop_lock_menu_set_clock_state(DesktopLockMenuView* lock_menu, bool enabled) {
    with_view_model(
        lock_menu->view,
        DesktopLockMenuViewModel * model,
        { model->clock_enabled = enabled; },
        true);
}

void desktop_lock_menu_set_brightness(DesktopLockMenuView* lock_menu, uint8_t brightness) {
    with_view_model(
        lock_menu->view,
        DesktopLockMenuViewModel * model,
        { model->brightness = brightness; },
        true);
}

void desktop_lock_menu_set_idx(DesktopLockMenuView* lock_menu, uint8_t idx) {
    furi_assert(idx < DesktopLockMenuIndexTotalCount);
    with_view_model(
        lock_menu->view, DesktopLockMenuViewModel * model, { model->idx = idx; }, true);
}

static void desktop_lock_menu_draw_tile(
    Canvas* canvas,
    uint8_t index,
    const Icon* icon,
    const char* label,
    bool active,
    bool selected) {
    const int32_t x = TILE_X0 + (index % TILE_COLUMNS) * (TILE_W + TILE_GAP_X);
    const int32_t y = TILE_Y0 + (index / TILE_COLUMNS) * (TILE_H + TILE_GAP_Y);

    canvas_set_color(canvas, ColorBlack);
    if(active) {
        canvas_draw_rbox(canvas, x, y, TILE_W, TILE_H, 3);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_draw_rframe(canvas, x, y, TILE_W, TILE_H, 3);
    }

    // Selection is a second frame inside the tile, so it works for both tile styles
    if(selected) {
        canvas_draw_rframe(canvas, x + 1, y + 1, TILE_W - 2, TILE_H - 2, 2);
    }

    canvas_draw_icon(canvas, x + (TILE_W - TILE_ICON_SIZE) / 2, y + 2, icon);
    canvas_draw_str_aligned(
        canvas, x + TILE_W / 2, y + TILE_H - 3, AlignCenter, AlignBottom, label);
    canvas_set_color(canvas, ColorBlack);
}

void desktop_lock_menu_draw_callback(Canvas* canvas, void* model) {
    DesktopLockMenuViewModel* m = model;

    canvas_set_font(canvas, FontSecondary);

    char brightness[8];
    snprintf(brightness, sizeof(brightness), "%u%%", m->brightness);

    desktop_lock_menu_draw_tile(
        canvas,
        DesktopLockMenuIndexLock,
        &I_CcLock_13x13,
        "Lock",
        false,
        m->idx == DesktopLockMenuIndexLock);
    desktop_lock_menu_draw_tile(
        canvas,
        DesktopLockMenuIndexSound,
        m->stealth_mode ? &I_CcSoundOff_13x13 : &I_CcSoundOn_13x13,
        m->stealth_mode ? "Muted" : "Sound",
        !m->stealth_mode,
        m->idx == DesktopLockMenuIndexSound);
    desktop_lock_menu_draw_tile(
        canvas,
        DesktopLockMenuIndexBluetooth,
        &I_CcBluetooth_13x13,
        m->bt_available ? "BLE" : "No BLE",
        m->bt_available && m->bt_enabled,
        m->idx == DesktopLockMenuIndexBluetooth);
    desktop_lock_menu_draw_tile(
        canvas,
        DesktopLockMenuIndexBrightness,
        &I_CcBrightness_13x13,
        brightness,
        false,
        m->idx == DesktopLockMenuIndexBrightness);
    desktop_lock_menu_draw_tile(
        canvas,
        DesktopLockMenuIndexClock,
        &I_CcClock_13x13,
        "Clock",
        m->clock_enabled,
        m->idx == DesktopLockMenuIndexClock);
    desktop_lock_menu_draw_tile(
        canvas,
        DesktopLockMenuIndexPower,
        &I_CcPower_13x13,
        "Power",
        false,
        m->idx == DesktopLockMenuIndexPower);
}

View* desktop_lock_menu_get_view(DesktopLockMenuView* lock_menu) {
    furi_assert(lock_menu);
    return lock_menu->view;
}

static uint8_t desktop_lock_menu_move(uint8_t idx, InputKey key) {
    const uint8_t rows = DesktopLockMenuIndexTotalCount / TILE_COLUMNS;
    uint8_t column = idx % TILE_COLUMNS;
    uint8_t row = idx / TILE_COLUMNS;

    if(key == InputKeyLeft) {
        column = (column + TILE_COLUMNS - 1) % TILE_COLUMNS;
    } else if(key == InputKeyRight) {
        column = (column + 1) % TILE_COLUMNS;
    } else if(key == InputKeyUp) {
        row = (row + rows - 1) % rows;
    } else if(key == InputKeyDown) {
        row = (row + 1) % rows;
    }

    return row * TILE_COLUMNS + column;
}

static DesktopEvent desktop_lock_menu_get_event(const DesktopLockMenuViewModel* model) {
    switch(model->idx) {
    case DesktopLockMenuIndexSound:
        return model->stealth_mode ? DesktopLockMenuEventStealthModeOff :
                                     DesktopLockMenuEventStealthModeOn;
    case DesktopLockMenuIndexBluetooth:
        return model->bt_enabled ? DesktopLockMenuEventBtOff : DesktopLockMenuEventBtOn;
    case DesktopLockMenuIndexBrightness:
        return DesktopLockMenuEventBrightnessNext;
    case DesktopLockMenuIndexClock:
        return model->clock_enabled ? DesktopLockMenuEventClockOff : DesktopLockMenuEventClockOn;
    case DesktopLockMenuIndexPower:
        return DesktopLockMenuEventPowerOff;
    case DesktopLockMenuIndexLock:
    default:
        return DesktopLockMenuEventLock;
    }
}

bool desktop_lock_menu_input_callback(InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    DesktopLockMenuView* lock_menu = context;

    if(event->key == InputKeyBack) {
        return false;
    }

    if(event->key == InputKeyOk) {
        if(event->type == InputTypeShort) {
            DesktopLockMenuViewModel* model = view_get_model(lock_menu->view);
            const DesktopEvent desktop_event = desktop_lock_menu_get_event(model);
            const bool enabled = model->idx != DesktopLockMenuIndexBluetooth ||
                                 model->bt_available;
            view_commit_model(lock_menu->view, false);

            if(enabled) lock_menu->callback(desktop_event, lock_menu->context);
        }
    } else if((event->type == InputTypeShort) || (event->type == InputTypeRepeat)) {
        with_view_model(
            lock_menu->view,
            DesktopLockMenuViewModel * model,
            { model->idx = desktop_lock_menu_move(model->idx, event->key); },
            true);
    }

    return true;
}

DesktopLockMenuView* desktop_lock_menu_alloc(void) {
    DesktopLockMenuView* lock_menu = malloc(sizeof(DesktopLockMenuView));
    lock_menu->view = view_alloc();
    view_allocate_model(lock_menu->view, ViewModelTypeLocking, sizeof(DesktopLockMenuViewModel));
    view_set_context(lock_menu->view, lock_menu);
    view_set_draw_callback(lock_menu->view, (ViewDrawCallback)desktop_lock_menu_draw_callback);
    view_set_input_callback(lock_menu->view, desktop_lock_menu_input_callback);

    return lock_menu;
}

void desktop_lock_menu_free(DesktopLockMenuView* lock_menu_view) {
    furi_assert(lock_menu_view);

    view_free(lock_menu_view->view);
    free(lock_menu_view);
}
