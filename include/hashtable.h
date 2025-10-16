#pragma once

#include <cstdint>

// 哈希表节点（需嵌入到用户数据结构中）
struct HNode {
    HNode *next = nullptr;
    uint64_t hcode = 0;
};

// 固定大小的哈希表（内部实现，用户不直接操作）
struct HTab {
    HNode **tab = nullptr;  // 哈希桶数组
    size_t mask = 0;        // 索引掩码（值为 size-1，size 是2的幂，用于快速计算索引）
    size_t size = 0;        // 当前表中存储的节点总数
};

// 哈希表对外接口（封装渐进式重哈希逻辑）
struct HMap {
    HTab newer;
    HTab older;
    size_t migrate_pos = 0;
};

HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));

void hm_insert(HMap *hmap, HNode *node);

HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));

void hm_clear(HMap *hmap);

size_t hm_size(const HMap *hmap);
