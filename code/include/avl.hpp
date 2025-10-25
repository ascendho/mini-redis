#pragma once

#include <cstdint>

// AVL 树
struct AVLNode {
    AVLNode *parent = nullptr;
    AVLNode *left = nullptr;
    AVLNode *right = nullptr;
    uint32_t height = 0;        // 子树高度
    uint32_t cnt = 0;           // 子树大小（以当前节点为根的子树中包含的节点总数，包括自身）
};

// 初始化节点
inline void avl_init(AVLNode *node) {
    node->left = node->right = node->parent = nullptr;
    node->height = 1;
    node->cnt = 1;
}

// 辅助函数
inline uint32_t avl_height(const AVLNode *node) { return node ? node->height : 0; }
inline uint32_t avl_cnt(const AVLNode *node) { return node ? node->cnt : 0; }

// 对外接口
AVLNode *avl_fix(AVLNode *node);

AVLNode *avl_del(const AVLNode *node);
