#ifndef UI_POETRY_DATA_H
#define UI_POETRY_DATA_H

#include <stdint.h>

#define MAX_POEMS 300
#define MAX_POEM_TITLE_LEN 64
#define MAX_POEM_AUTHOR_LEN 32
#define MAX_POEM_SUBTITLE_LEN 32
#define MAX_POEM_CONTENT_LEN 1024

typedef struct {
    char title[MAX_POEM_TITLE_LEN];
    char author[MAX_POEM_AUTHOR_LEN];
    char subtitle[MAX_POEM_SUBTITLE_LEN];
    char content[MAX_POEM_CONTENT_LEN];
} poem_t;

void ui_poetry_data_init(void);
uint16_t ui_poetry_get_count(void);
poem_t *ui_poetry_get_poem(uint16_t index);

#endif // UI_POETRY_DATA_H
