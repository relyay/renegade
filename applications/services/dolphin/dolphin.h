/**
 * @file dolphin.h
 * Dolphin API compatibility layer.
 *
 * The dolphin service has been removed from this firmware. Every function
 * here is a no-op that is kept only for compatibility with third-party
 * applications. Do not use it in new code.
 */
#pragma once

#include <stdbool.h>
#include <core/pubsub.h>

#include "helpers/dolphin_deed.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_DOLPHIN "dolphin"

typedef struct Dolphin Dolphin;

typedef struct {
    uint32_t icounter;
    uint32_t butthurt;
    uint64_t timestamp;
    uint8_t level;
    bool level_up_is_pending;
} DolphinStats;

typedef struct {
    bool happy_mode;
} DolphinSettings;

typedef enum {
    DolphinPubsubEventUpdate,
} DolphinPubsubEvent;

/** Deed complete notification. Does nothing. */
void dolphin_deed(DolphinDeed deed);

void dolphin_get_settings(Dolphin* dolphin, DolphinSettings* settings);

void dolphin_set_settings(Dolphin* dolphin, DolphinSettings* settings);

/** Retrieve dolphin stats. Always returns zeroed stats. */
DolphinStats dolphin_stats(Dolphin* dolphin);

/** Does nothing. */
void dolphin_flush(Dolphin* dolphin);

void dolphin_upgrade_level(Dolphin* dolphin);

FuriPubSub* dolphin_get_pubsub(Dolphin* dolphin);

#ifdef __cplusplus
}
#endif
