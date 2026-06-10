#include "ui_poetry_data.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static poem_t g_poems[MAX_POEMS];
static uint16_t g_poem_count = 0;

// 简单的 JSON 解析：提取字符串值
static int extract_json_string(const char *json, const char *key, char *out, size_t out_len) {
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\":", key);
    const char *pos = strstr(json, search_key);
    if (!pos) return 0;

    pos = strchr(pos, ':');
    if (!pos) return 0;
    pos++;

    while (*pos && (*pos == ' ' || *pos == '\t')) pos++;

    if (*pos != '"') return 0;
    pos++;

    size_t i = 0;
    while (*pos && *pos != '"' && i < out_len - 1) {
        if (*pos == '\\' && *(pos + 1) == 'n') {
            out[i++] = '\n';
            pos += 2;
        } else if (*pos == '\\' && *(pos + 1) == '"') {
            out[i++] = '"';
            pos += 2;
        } else {
            out[i++] = *pos++;
        }
    }
    out[i] = '\0';
    return 1;
}

static void parse_json_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        printf("Cannot open JSON file: %s\n", filepath);
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(file_size + 1);
    if (!content) {
        fclose(file);
        return;
    }

    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0';
    fclose(file);

    // 查找 chapters 数组
    const char *chapters_start = strstr(content, "\"chapters\"");
    if (!chapters_start) {
        printf("No 'chapters' found in JSON\n");
        free(content);
        return;
    }

    const char *array_start = strchr(chapters_start, '[');
    if (!array_start) {
        free(content);
        return;
    }

    const char *array_end = strchr(array_start, ']');
    if (!array_end) {
        free(content);
        return;
    }

    uint16_t poem_count = 0;
    const char *pos = array_start + 1;

    // 遍历整个 chapters 数组
    while (pos < array_end && poem_count < MAX_POEMS) {
        // 找下一个 poems 数组
        const char *poems_key = strstr(pos, "\"poems\"");
        if (!poems_key || poems_key > array_end) break;

        const char *poems_array = strchr(poems_key, '[');
        if (!poems_array || poems_array > array_end) break;

        const char *poems_array_end = strchr(poems_array, ']');
        if (!poems_array_end || poems_array_end > array_end) break;

        // 遍历这个 poems 数组中的所有诗词
        const char *poem_pos = poems_array + 1;
        while (poem_pos < poems_array_end && poem_count < MAX_POEMS) {
            const char *poem_start = strchr(poem_pos, '{');
            if (!poem_start || poem_start >= poems_array_end) break;

            const char *poem_end = strchr(poem_start, '}');
            if (!poem_end || poem_end >= poems_array_end) break;

            // 提取这个诗词对象
            size_t obj_len = poem_end - poem_start + 1;
            char obj_str[MAX_POEM_CONTENT_LEN];
            if (obj_len >= sizeof(obj_str)) obj_len = sizeof(obj_str) - 1;
            strncpy(obj_str, poem_start, obj_len);
            obj_str[obj_len] = '\0';

            // 从对象中提取字段
            if (extract_json_string(obj_str, "title", g_poems[poem_count].title, MAX_POEM_TITLE_LEN) &&
                extract_json_string(obj_str, "author", g_poems[poem_count].author, MAX_POEM_AUTHOR_LEN) &&
                extract_json_string(obj_str, "content", g_poems[poem_count].content, MAX_POEM_CONTENT_LEN)) {
                // subtitle 为可选字段
                extract_json_string(obj_str, "subtitle", g_poems[poem_count].subtitle, MAX_POEM_SUBTITLE_LEN);
                poem_count++;
            }

            poem_pos = poem_end + 1;
        }

        pos = poems_array_end + 1;
    }

    g_poem_count = poem_count;
    free(content);
    printf("Loaded %d poems from JSON\n", g_poem_count);
}

void ui_poetry_data_init(void) {
    if (g_poem_count == 0) {
        // 尝试多个可能的路径
        parse_json_file("../src/ui/assets/poetry.json");
        if (g_poem_count == 0) {
            parse_json_file("./src/ui/assets/poetry.json");
        }
        if (g_poem_count == 0) {
            parse_json_file("src/ui/assets/poetry.json");
        }
    }
}

uint16_t ui_poetry_get_count(void) {
    return g_poem_count;
}

poem_t *ui_poetry_get_poem(uint16_t index) {
    if (index >= g_poem_count) return NULL;
    return &g_poems[index];
}


