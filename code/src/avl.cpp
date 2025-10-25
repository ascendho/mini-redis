#include <cassert>
#include "../include/avl.hpp"

// 取最大值函数
static uint32_t max(uint32_t lhs, uint32_t rhs) {
    return lhs < rhs ? rhs : lhs;
}

// 更新节点的高度和子树大小
static void avl_update(AVLNode *node) {
    // 1（自身） + max（左子树高度，右子树高度）
    node->height = 1 + max(avl_height(node->left), avl_height(node->right));

    // 自己 + 左子树节点数 + 右子树节点数
    node->cnt = 1 + avl_cnt(node->left) + avl_cnt(node->right);
}

// 左旋（当右子树过高时使用）
static AVLNode *rot_left(AVLNode *node) {
    AVLNode *parent = node->parent;
    AVLNode *new_node = node->right;
    AVLNode *inner = new_node->left;

    node->right = inner;
    if (inner) {
        inner->parent = node;
    }

    new_node->parent = parent;
    new_node->left = node;
    node->parent = new_node;

    avl_update(node);
    avl_update(new_node);

    return new_node;
}

// 右旋（当左子树过高时使用）
static AVLNode *rot_right(AVLNode *node) {
    AVLNode *parent = node->parent;
    AVLNode *new_node = node->left;
    AVLNode *inner = new_node->right;

    node->left = inner;
    if (inner) {
        inner->parent = node;
    }

    new_node->parent = parent;
    new_node->right = node;
    node->parent = new_node;

    avl_update(node);
    avl_update(new_node);

    return new_node;
}

// 修复左子树比右子树高 2 的情况
static AVLNode *avl_fix_left(AVLNode *node) {
    // LR型：左子树的右子树更高
    if (avl_height(node->left->left) < avl_height(node->left->right)) {
        // 先左旋，转换为 LL 型
        node->left = rot_left(node->left);
    }

    // LL型：直接右旋当前节点
    return rot_right(node);
}

// 修复右子树比左子树高 2 的情况
static AVLNode *avl_fix_right(AVLNode *node) {
    // RL型：右子树的左子树更高
    if (avl_height(node->right->right) < avl_height(node->right->left)) {
        // 先右旋，转换为 RR 型
        node->right = rot_right(node->right);
    }

    // RR型：直接左旋当前节点
    return rot_left(node);
}

// 从当前节点向上递归修复，直到根节点
AVLNode *avl_fix(AVLNode *node) {
    while (true) {
        /*
         * from 是一个指向指针的指针（二级指针）：
         * 1. 如果有父节点，from 指向父节点中指向当前节点的那个指针
         * 2. 如果没有父节点，from 就指向局部变量 node
         * 无论旋转后新根是谁，都能正确更新到父节点或局部变量
         *
         */
        AVLNode **from = &node;
        AVLNode *parent = node->parent;
        if (parent) {
            from = parent->left == node ? &parent->left : &parent->right;
        }

        // 更新高度和节点计数
        avl_update(node);

        // 检查并修复不平衡
        uint32_t l = avl_height(node->left);
        uint32_t r = avl_height(node->right);
        if (l == r + 2) {
            *from = avl_fix_left(node);
        } else if (l + 2 == r) {
            *from = avl_fix_right(node);
        }

        // 到达根节点，返回树的根
        if (!parent) {
            return *from;
        }

        // 因为当前节点的高度可能改变，需要继续向上检查，直到根节点为止
        node = parent;
    }
}

// 删除只有 0 或 1 个子节点的节点
static AVLNode *avl_del_easy(const AVLNode *node) {
    // 最多有一个子节点
    assert(!node->left || !node->right);
    AVLNode *child = node->left ? node->left : node->right;
    AVLNode *parent = node->parent;

    // 更新子节点的父指针
    if (child) {
        child->parent = parent;
    }

    /*
     * 将子节点连接到祖父：
     * 1. 如果删除的是根节点，直接返回子节点作为新根
     * 2. 否则找到父节点中指向被删除节点的指针，改为指向子节点
     *
     */
    if (!parent) {
        return child;       // 删除的是根节点
    }
    AVLNode **from = parent->left == node ? &parent->left : &parent->right;
    *from = child;

    // 重新平衡
    return avl_fix(parent);
}

// 删除节点，并返回新树的根
AVLNode *avl_del(const AVLNode *node) {
    // 如果节点有0或1个子节点 → 直接删除
    if (!node->left || !node->right) {
        return avl_del_easy(node);
    }

    // 如果节点有2个子节点 → 用后继节点替换
    AVLNode *victim = node->right;
    while (victim->left) {
        victim = victim->left;
    }

    // 删除后继节点
    AVLNode *root = avl_del_easy(victim);

    // 用后继替换被删除节点
    *victim = *node;                     // 复制 left, right, parent 三个指针

    // 更新子节点的父指针
    if (victim->left) {
        victim->left->parent = victim;
    }
    if (victim->right) {
        victim->right->parent = victim;
    }

    /*
     * 将 victim 连接到原 node 的父节点：
     * 1. 如果 node 有父节点，将父节点的指针指向 victim
     * 2. 如果 node 是根，更新 root 指向 victim
     *
     */
    AVLNode **from = &root;
    AVLNode *parent = node->parent;
    if (parent) {
        from = parent->left == node ? &parent->left : &parent->right;
    }
    *from = victim;

    // 返回新根
    return root;
}
