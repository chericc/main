
/**
 * @brief: 用于测试手写代码的纯 C　链表样例。
 * @date: 2020.12.14
 */

#pragma once

typedef struct NODE {
    int data;
    struct NODE* next;
} NODE;

/************ BASE ***********/

/**
 * 使用提供的数组，生成一个带头结点的链表
 */
NODE* list_gen(int arr[], int size);

/**
 * 打印链表，包括头节点
 */
void list_print(const NODE* list);

/************ BASE OP ***********/

/**
 * 这一部分的链表基本操作要求熟练掌握。
 * 算法要求：清晰，简洁。
 */

/**
 * 逆置链表
 */
NODE* list_reverse_bare(NODE* list);
NODE* list_reverse(NODE* list);

/**
 * 连接链表（第二个链表连接到第一个链表尾部）
 */
NODE* list_append_bare(NODE* lista, NODE* listb);
NODE* list_append(NODE* lista, NODE* listb);

/**
 * 合并链表（升序链表合并之后保持升序）
 */
NODE* list_merge_bare(NODE* lista, NODE* listb);
NODE* list_merge(NODE* lista, NODE* listb);

/************ FUNN ***********/

/**
 *  1 2 3 4 5 -->
 *  1 5 2 4 3
 *
 *  1 2 3 4 5 6 -->
 *  1 6 2 5 3 4
 */
NODE* list_eg1(NODE* list);