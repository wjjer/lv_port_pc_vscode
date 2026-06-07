#ifndef UI_ALARM_PERSIST_H
#define UI_ALARM_PERSIST_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t hour;
    uint8_t minute;
    bool active;
    bool repeat;
    char label[64];
    char ringtone[64];
} alarm_persist_t;

// Save all alarms to storage
// Returns true on success, false on failure
bool ui_alarm_persist_save(const alarm_persist_t *alarms, int count);

// Load alarms from storage
// Returns number of alarms loaded (0-MAX_ALARMS)
int ui_alarm_persist_load(alarm_persist_t *alarms, int max_count);

// Delete all alarm data from storage
bool ui_alarm_persist_clear(void);

#endif // UI_ALARM_PERSIST_H
