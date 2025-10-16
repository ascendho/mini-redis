#include <cassert>
#include <cstdlib>
#include "../include/hashtable.h"

// n 必须是2的幂
static void h_init(HTab *htab, size_t n) {
    // 确保 n 是大于 0 的 2 的幂次方（如 2、4、8、16 等）
    assert(n > 0 && ((n - 1) & n) == 0);
    htab->tab = static_cast<HNode **>(calloc(n, sizeof(HNode *)));
    htab->mask = n - 1;
    htab->size = 0;
}

// 哈希表插入操作
static void h_insert(HTab *htab, HNode *node) {
    size_t pos = node->hcode & htab->mask;
    HNode *next = htab->tab[pos];
    node->next = next;
    htab->tab[pos] = node;
    htab->size++;
}

// 哈希表查找子程序。
// 请注意返回值：它返回指向目标节点的前驱指针的地址，
// 该地址可用于删除目标节点
static HNode **h_lookup(const HTab *htab, HNode *key, bool (*eq)(HNode *, HNode *)) {
    if (!htab->tab) {
        return nullptr;
    }

    size_t pos = key->hcode & htab->mask;
    // 指向目标节点的前驱指针
    HNode **from = &htab->tab[pos];
    for (HNode *cur; (cur = *from) != nullptr; from = &cur->next) {
        if (cur->hcode == key->hcode && eq(cur, key)) {
            // 可能是节点，也可能是插槽（slot）
            return from;
        }
    }
    return nullptr;
}

// 从链表中移除节点
static HNode *h_detach(HTab *htab, HNode **from) {
    // 目标节点
    HNode *node = *from;
    // 更新前驱节点指向目标节点
    *from = node->next;
    htab->size--;
    return node;
}

// 固定工作量
constexpr size_t k_rehashing_work = 128;

static void hm_help_rehashing(HMap *hmap) {
    size_t nwork = 0;
    while (nwork < k_rehashing_work && hmap->older.size > 0) {
        // 寻找非空插槽
        HNode **from = &hmap->older.tab[hmap->migrate_pos];
        // 空插槽
        if (!*from) {
            hmap->migrate_pos++;
            continue;
        }
        // 将链表的第一个节点移到新表中
        h_insert(&hmap->newer, h_detach(&hmap->older, from));
        nwork++;
    }
    // 若（旧表节点）迁移完成，则释放旧表
    if (hmap->older.size == 0 && hmap->older.tab) {
        free(hmap->older.tab);
        hmap->older = HTab{};
    }
}

static void hm_trigger_rehashing(HMap *hmap) {
    assert(hmap->older.tab == nullptr);
    // (newer, older) <- (new_table, newer)
    hmap->older = hmap->newer;
    h_init(&hmap->newer, (hmap->newer.mask + 1) * 2);
    hmap->migrate_pos = 0;
}

HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *)) {
    hm_help_rehashing(hmap);
    HNode **from = h_lookup(&hmap->newer, key, eq);
    if (!from) {
        from = h_lookup(&hmap->older, key, eq);
    }
    return from ? *from : nullptr;
}

constexpr size_t k_max_load_factor = 8;

void hm_insert(HMap *hmap, HNode *node) {
    // 若（新表）为空，则对其进行初始化
    if (!hmap->newer.tab) {
        h_init(&hmap->newer, 4);
    }

    // 总是插入到新表中
    h_insert(&hmap->newer, node);

    if (!hmap->older.tab) {
        // 检查是否需要重哈希
        size_t shreshold = (hmap->newer.mask + 1) * k_max_load_factor;
        if (hmap->newer.size >= shreshold) {
            hm_trigger_rehashing(hmap);
        }
    }

    // 迁移部分节点
    hm_help_rehashing(hmap);
}

HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *)) {
    hm_help_rehashing(hmap);
    if (HNode **from = h_lookup(&hmap->newer, key, eq)) {
        return h_detach(&hmap->newer, from);
    }
    if (HNode **from = h_lookup(&hmap->older, key, eq)) {
        return h_detach(&hmap->older, from);
    }
    return nullptr;
}

void hm_clear(HMap *hmap) {
    free(hmap->newer.tab);
    free(hmap->older.tab);
    *hmap = HMap{};
}

size_t hm_size(const HMap *hmap) {
    return hmap->newer.size + hmap->older.size;
}
