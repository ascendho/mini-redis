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
    size_t mask = 0;        // 索引掩码（用于快速计算索引，mask+1 等于哈希表容量）
    size_t size = 0;        // 当前表中存储的节点总数
};

/*
 * 哈希表掩码（mask）计算索引的示例与优势说明
 * 1. 示例参数定义
 *    - 哈希表容量 N = 8（需为2的幂）
 *    - 掩码 mask = N - 1 = 7，二进制表示为 0b111
 *    - 某节点哈希值 hcode = 25，二进制表示为 0b11001
 *
 * 2. 掩码计算索引过程（hcode & mask）
 *    - 计算公式：hcode & mask = 25 & 7
 *    - 二进制底层运算：0b11001（25） & 0b00111（7） = 0b00001（1）
 *    - 计算结果：索引值为 1
 *
 * 3. 与取模运算（hcode % N）的对比
 *    - 取模计算公式：hcode % N = 25 % 8
 *    - 取模结果：1（与掩码计算结果完全一致）
 *
 * 4. 优先使用掩码计算的原因
 *    - 1. 性能更优：位运算（&）是计算机硬件直接支持的基础操作，执行速度远快于取模运算（%）
 *    - 2. 索引安全：mask 二进制为“全1”形式（如7对应 0b111），
 *                 hcode & mask 的结果必然落在 0～mask（即 0～N-1）范围内，
 *                 刚好匹配哈希桶数组的索引区间，不会出现索引越界问题
 */

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
