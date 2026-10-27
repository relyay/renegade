/**
 * @file dolphin.c
 * No-op replacement for the removed dolphin service.
 *
 * The leveling system, deeds and their persistent state are gone. This
 * module only keeps the public dolphin API alive, so that third-party
 * applications built against the stock SDK keep loading and running.
 */
#include "dolphin.h"

#include <furi.h>

struct Dolphin {
    FuriPubSub* pubsub;
};

void dolphin_on_system_start(void) {
    Dolphin* dolphin = malloc(sizeof(Dolphin));
    dolphin->pubsub = furi_pubsub_alloc();
    furi_record_create(RECORD_DOLPHIN, dolphin);
}

void dolphin_deed(DolphinDeed deed) {
    UNUSED(deed);
}

void dolphin_get_settings(Dolphin* dolphin, DolphinSettings* settings) {
    furi_check(dolphin);
    furi_check(settings);
    settings->happy_mode = false;
}

void dolphin_set_settings(Dolphin* dolphin, DolphinSettings* settings) {
    furi_check(dolphin);
    furi_check(settings);
}

DolphinStats dolphin_stats(Dolphin* dolphin) {
    furi_check(dolphin);
    return (DolphinStats){0};
}

void dolphin_flush(Dolphin* dolphin) {
    furi_check(dolphin);
}

void dolphin_upgrade_level(Dolphin* dolphin) {
    furi_check(dolphin);
}

FuriPubSub* dolphin_get_pubsub(Dolphin* dolphin) {
    furi_check(dolphin);
    return dolphin->pubsub;
}

DolphinApp dolphin_deed_get_app(DolphinDeed deed) {
    UNUSED(deed);
    return DolphinAppMAX;
}

uint8_t dolphin_deed_get_app_limit(DolphinApp app) {
    UNUSED(app);
    return 0;
}

uint8_t dolphin_deed_get_weight(DolphinDeed deed) {
    UNUSED(deed);
    return 0;
}
