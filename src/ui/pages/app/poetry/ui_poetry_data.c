#include "ui_poetry_data.h"
#include <string.h>
#include <stdio.h>

poetry_db_t g_poetry_db = {0};

// 预定义的诗词数据（从唐诗三百首.txt精选的代表性诗词）
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

    // 导入预定义数据
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
