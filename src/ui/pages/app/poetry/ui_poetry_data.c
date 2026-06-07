#include "ui_poetry_data.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

poetry_db_t g_poetry_db = {0};

// 从文件中读取诗词数据
static void parse_poetry_file(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (file == NULL) {
        // 文件打开失败，使用硬编码数据
        return;
    }

    char line[512];
    uint16_t poem_count = 0;
    char current_title[MAX_POEM_TITLE_LEN] = {0};
    char current_author[MAX_POEM_AUTHOR_LEN] = {0};
    char current_content[MAX_POEM_CONTENT_LEN] = {0};
    uint8_t current_stage = 0;
    int content_lines = 0;

    while (fgets(line, sizeof(line), file) && poem_count < MAX_POEMS) {
        // 移除行尾的换行符
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        // 跳过空行
        if (len == 0) {
            // 如果已经有诗词内容，保存当前诗词
            if (current_title[0] != '\0' && current_content[0] != '\0') {
                strncpy(g_poetry_db.poems[poem_count].title, current_title, MAX_POEM_TITLE_LEN - 1);
                strncpy(g_poetry_db.poems[poem_count].author, current_author, MAX_POEM_AUTHOR_LEN - 1);
                strncpy(g_poetry_db.poems[poem_count].content, current_content, MAX_POEM_CONTENT_LEN - 1);
                strcpy(g_poetry_db.poems[poem_count].notes, "");
                g_poetry_db.poems[poem_count].stage = current_stage;
                g_poetry_db.stage_counts[current_stage]++;

                poem_count++;
                memset(current_title, 0, sizeof(current_title));
                memset(current_author, 0, sizeof(current_author));
                memset(current_content, 0, sizeof(current_content));
                content_lines = 0;
            }
            continue;
        }

        // 检查诗词标题行 格式: 序号作者：标题
        if (isdigit(line[0]) && line[1] >= '0' && line[1] <= '9') {
            // 保存上一首诗词
            if (current_title[0] != '\0' && current_content[0] != '\0') {
                strncpy(g_poetry_db.poems[poem_count].title, current_title, MAX_POEM_TITLE_LEN - 1);
                strncpy(g_poetry_db.poems[poem_count].author, current_author, MAX_POEM_AUTHOR_LEN - 1);
                strncpy(g_poetry_db.poems[poem_count].content, current_content, MAX_POEM_CONTENT_LEN - 1);
                strcpy(g_poetry_db.poems[poem_count].notes, "");
                g_poetry_db.poems[poem_count].stage = current_stage;
                g_poetry_db.stage_counts[current_stage]++;

                poem_count++;
                memset(current_content, 0, sizeof(current_content));
                content_lines = 0;
            }

            // 解析新的诗词行 格式: "010杜甫：佳人"
            char *colon = strchr(line, '：');
            if (colon != NULL) {
                // 分离作者和标题
                strncpy(current_author, colon - 4, 4); // 取作者名
                current_author[4] = '\0';
                strncpy(current_title, colon + 1, MAX_POEM_TITLE_LEN - 1);

                // 根据编号估计阶段（10-50小学，51-200初中，201+高中）
                int poem_id = atoi(line);
                if (poem_id <= 50) {
                    current_stage = 0; // 小学
                } else if (poem_id <= 200) {
                    current_stage = 1; // 初中
                } else {
                    current_stage = 2; // 高中
                }
            }
            continue;
        }

        // 诗词内容行
        if (current_title[0] != '\0') {
            if (content_lines > 0) {
                strncat(current_content, "\n", MAX_POEM_CONTENT_LEN - strlen(current_content) - 1);
            }
            strncat(current_content, line, MAX_POEM_CONTENT_LEN - strlen(current_content) - 1);
            content_lines++;
        }
    }

    // 保存最后一首诗词
    if (current_title[0] != '\0' && current_content[0] != '\0' && poem_count < MAX_POEMS) {
        strncpy(g_poetry_db.poems[poem_count].title, current_title, MAX_POEM_TITLE_LEN - 1);
        strncpy(g_poetry_db.poems[poem_count].author, current_author, MAX_POEM_AUTHOR_LEN - 1);
        strncpy(g_poetry_db.poems[poem_count].content, current_content, MAX_POEM_CONTENT_LEN - 1);
        strcpy(g_poetry_db.poems[poem_count].notes, "");
        g_poetry_db.poems[poem_count].stage = current_stage;
        g_poetry_db.stage_counts[current_stage]++;
        poem_count++;
    }

    fclose(file);
    g_poetry_db.count = poem_count;
}

// 预定义的诗词数据（备选方案，当文件读取失败时使用）
static const struct {
    const char *title;
    const char *author;
    const char *content;
    const char *notes;
    uint8_t stage;
} poetry_data[] = {
    {
        "静夜思",
        "【唐】李白",
        "床前明月光，\n疑是地上霜。\n举头望明月，\n低头思故乡。",
        "【赏析】游子孤身在外的深夜思乡之情，语言质朴，流传千古。",
        0
    },
    {
        "春晓",
        "【唐】孟浩然",
        "春眠不觉晓，\n处处闻啼鸟。\n夜来风雨声，\n花落知多少？",
        "【赏析】用拟人化手法写春天的早晨，清新自然。",
        0
    },
    {
        "登鹳雀楼",
        "【唐】王之涣",
        "白日依山尽，\n黄河入海流。\n欲穷千里目，\n更上一层楼。",
        "【赏析】表现登高望远的雄心和乐观向上的精神。",
        0
    },
    {
        "咏鹅",
        "【唐】骆宾王",
        "鹅鹅鹅，\n曲项向天歌。\n白毛浮绿水，\n红掌拨清波。",
        "【赏析】儿童诗歌的代表作，形象生动。",
        0
    },
    {
        "悯农（其一）",
        "【唐】李绅",
        "春种一粒粟，\n秋收万颗子。\n四海无闲田，\n农夫犹饿死。",
        "【赏析】讽刺社会不公，同情农民苦难。",
        1
    },
    {
        "游子吟",
        "【唐】孟郊",
        "慈母手中线，\n游子身上衣。\n临行密密缝，\n意恐迟迟归。\n谁言寸草心，\n报得三春辉？",
        "【赏析】歌颂母爱的伟大和无私，感人至深。",
        1
    },
    {
        "绝句",
        "【唐】杜甫",
        "两个黄鹂鸣翠柳，\n一行白鹭上青天。\n窗含西岭千秋雪，\n门泊东吴万里船。",
        "【赏析】景物色彩绚丽，构成和谐的图画。",
        1
    },
    {
        "蜀道难",
        "【唐】李白",
        "噫吁嘻，危乎高哉！\n蜀道之难难于上青天！",
        "【赏析】夸张手法表现蜀道的艰险，气势磅礴。",
        2
    },
    {
        "琵琶行",
        "【唐】白居易",
        "浔阳江头夜送客，\n枫叶荻花秋瑟瑟。\n主人下马客在船，\n举酒欲饮无管弦。",
        "【赏析】长篇叙事诗，同情被遗弃的妓女。",
        2
    },
    {
        "登高",
        "【唐】杜甫",
        "风急天高猿啸哀，\n渚清沙白鸟飞回。\n无边落木萧萧下，\n不尽长江滚滚来。",
        "【赏析】秋景苍凉，表现诗人晚年的悲伤。",
        2
    },
};

void ui_poetry_data_init(void) {
    memset(&g_poetry_db, 0, sizeof(poetry_db_t));

    // 尝试从文件读取诗词
    parse_poetry_file("src/ui/assets/唐诗三百首.txt");

    // 如果文件读取失败或数据为空，使用硬编码数据
    if (g_poetry_db.count == 0) {
        uint16_t count = sizeof(poetry_data) / sizeof(poetry_data[0]);
        if (count > MAX_POEMS) count = MAX_POEMS;

        for (uint16_t i = 0; i < count; i++) {
            strncpy(g_poetry_db.poems[i].title, poetry_data[i].title, MAX_POEM_TITLE_LEN - 1);
            strncpy(g_poetry_db.poems[i].author, poetry_data[i].author, MAX_POEM_AUTHOR_LEN - 1);
            strncpy(g_poetry_db.poems[i].content, poetry_data[i].content, MAX_POEM_CONTENT_LEN - 1);
            strncpy(g_poetry_db.poems[i].notes, poetry_data[i].notes, MAX_POEM_NOTES_LEN - 1);
            g_poetry_db.poems[i].stage = poetry_data[i].stage;

            g_poetry_db.stage_counts[poetry_data[i].stage]++;
        }

        g_poetry_db.count = count;
    }
}

poem_t *ui_poetry_get_poem(uint16_t index) {
    if (index >= g_poetry_db.count) return NULL;
    return &g_poetry_db.poems[index];
}

uint16_t ui_poetry_get_stage_count(uint8_t stage) {
    if (stage >= 3) return 0;
    return g_poetry_db.stage_counts[stage];
}

poem_t *ui_poetry_get_stage_poem(uint8_t stage, uint16_t index) {
    if (stage >= 3) return NULL;

    uint16_t count = 0;
    for (uint16_t i = 0; i < g_poetry_db.count; i++) {
        if (g_poetry_db.poems[i].stage == stage) {
            if (count == index) return &g_poetry_db.poems[i];
            count++;
        }
    }
    return NULL;
}

