#ifndef UI_POETRY_DATA_H
#define UI_POETRY_DATA_H

#include <stdint.h>

#define MAX_POEMS 300
#define MAX_POEM_TITLE_LEN 64
#define MAX_POEM_AUTHOR_LEN 32
#define MAX_POEM_CONTENT_LEN 1024
#define MAX_POEM_NOTES_LEN 512

typedef struct {
    char title[MAX_POEM_TITLE_LEN];
    char author[MAX_POEM_AUTHOR_LEN];
    char content[MAX_POEM_CONTENT_LEN];
    char notes[MAX_POEM_NOTES_LEN];
    uint8_t stage; // 0: 小学, 1: 初中, 2: 高中
} poem_t;

typedef struct {
    poem_t poems[MAX_POEMS];
    uint16_t count;
    uint16_t stage_counts[3]; // 各阶段诗词数量
} poetry_db_t;

extern poetry_db_t g_poetry_db;

void ui_poetry_data_init(void);
poem_t *ui_poetry_get_poem(uint16_t index);
uint16_t ui_poetry_get_stage_count(uint8_t stage);
poem_t *ui_poetry_get_stage_poem(uint8_t stage, uint16_t index);

#endif // UI_POETRY_DATA_H
