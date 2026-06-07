#include "ui_alarm_persist.h"
#include <stdio.h>
#include <string.h>
#include <io.h>

#define ALARM_PERSIST_FILE "alarms.bin"
#define ALARM_PERSIST_MAGIC 0x414C524D  // "ALRM"
#define ALARM_PERSIST_VERSION 1

#pragma pack(push, 1)
typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t count;
} alarm_file_header_t;
#pragma pack(pop)

bool ui_alarm_persist_save(const alarm_persist_t *alarms, int count)
{
    if (alarms == NULL || count < 0 || count > 10)
        return false;

    FILE *f = fopen(ALARM_PERSIST_FILE, "wb");
    if (f == NULL)
        return false;

    alarm_file_header_t header;
    header.magic = ALARM_PERSIST_MAGIC;
    header.version = ALARM_PERSIST_VERSION;
    header.count = (uint32_t)count;

    if (fwrite(&header, sizeof(header), 1, f) != 1)
    {
        fclose(f);
        return false;
    }

    if (count > 0)
    {
        if (fwrite(alarms, sizeof(alarm_persist_t), count, f) != (size_t)count)
        {
            fclose(f);
            return false;
        }
    }

    fclose(f);
    return true;
}

int ui_alarm_persist_load(alarm_persist_t *alarms, int max_count)
{
    if (alarms == NULL || max_count <= 0)
        return -1;

    FILE *f = fopen(ALARM_PERSIST_FILE, "rb");
    if (f == NULL)
        return -1;

    alarm_file_header_t header;
    if (fread(&header, sizeof(header), 1, f) != 1)
    {
        fclose(f);
        return -1;
    }

    if (header.magic != ALARM_PERSIST_MAGIC || header.version != ALARM_PERSIST_VERSION)
    {
        fclose(f);
        return -1;
    }

    int count = (int)header.count;
    if (count < 0 || count > max_count)
    {
        fclose(f);
        return -1;
    }

    if (count > 0)
    {
        if (fread(alarms, sizeof(alarm_persist_t), count, f) != (size_t)count)
        {
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return count;
}

bool ui_alarm_persist_clear(void)
{
    // Try to remove the file; if it doesn't exist or removal succeeds, return true
    int result = remove(ALARM_PERSIST_FILE);
    return result == 0 || _access(ALARM_PERSIST_FILE, 0) != 0;
}
