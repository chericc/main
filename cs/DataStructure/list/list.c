#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

NODE *list_gen(int arr[], int size) {
    NODE *list = (NODE *)malloc(sizeof(NODE));
    memset (list, 0, sizeof(*list));
    NODE *tail = list;

    int i = 0;
    for (i = 0; i < size; ++i) {
        NODE *newnode = (NODE*)malloc(sizeof(NODE));
        memset (newnode, 0, sizeof(*newnode));
        newnode->data = arr[i];
        tail->next = newnode;
        tail = tail->next;
    }

    return list;
}

void list_print(const NODE *list) {
    printf ("list: %p\n", list);

    const NODE *it = list;
    while (it != NULL) {
        printf ("%d ", it->data);
        it = it->next;
    }
    printf ("\n");
    return ;
}

/**
 * 单链表的原地逆置必须用到三个指针指向连续的三个元素，前两个
 * 指针用于将待逆置的两个节点的指针反转，后一个指针用于进行遍
 * 历（因为前两个节点的指针已经被逆转了）。
 * 特点：不需要头节点辅助。中间节点为 NULL 时结束。
 */
NODE *list_reverse_bare(NODE *list) {
    /* 少于两个元素，不操作 */
    if (list == NULL || list->next == NULL) {
        return list;
    }
    
    NODE *r1 = list;
    NODE *r2 = list->next;
    NODE *r3 = list->next->next;
    r1->next = NULL; // 逆置后的尾节点
    while (r2) {
        r2->next = r1;
        r1 = r2;
        r2 = r3;
        if (r3) {
            r3 = r3->next;
        }
    }
    list = r1;
    return list;
}

NODE *list_reverse(NODE *list) {
    /* 少于两个元素，不操作 */
    if (list == NULL || list->next == NULL || list->next->next == NULL) {
        return list;
    }
    NODE *r = list_reverse_bare(list->next);
    list->next = r;
    return list;
}

NODE *list_append_bare(NODE *lista, NODE *listb) {
    NODE *it = lista;
    while (it->next) {
        it = it->next;
    }
    it->next = listb;
    return lista;
}

NODE *list_append(NODE *lista, NODE *listb) {
    list_append_bare (lista->next, listb->next);
    free (listb);
    return lista;
}

/**
 * 创建一个头节点（而不是头指针），用于指向
 * 合并后的链表。合并的过程是一个尾插的过程，
 * 要注意节点移动到尾部后指针清零。
 * 特点：头节点是临时节点。只改变原有链表的
 * 指针。
 * 
 * 扩展：
 * 头节点的作用：
 * 头节点在于让算法对空对象和非空对象具有处理的一致性。
 * 
 * 思考：这里为什么要用指向头节点的指针？
 * 建立一个空链表，并从尾部增加节点需要一个
 * 头节点，并需要一个游标。初始情况下游标指向
 * 头节点。
 * 这里的头节点不能用指针代替，是因为游标指针
 * 必须指向头节点。
 */
NODE *list_merge_bare(NODE *lista, NODE *listb) {
    NODE head = {};
    NODE *tail = &head;
    while (1) {
        if (lista != NULL && listb != NULL) {
            if (lista->data <= listb->data) {
                tail->next = lista;
                tail = tail->next;
                lista = lista->next;
                tail->next = NULL;
            } else {
                tail->next = listb;
                tail = tail->next;
                listb = listb->next;
                tail->next = NULL;
            }
        }
        else if (lista != NULL) {
            tail->next = lista;
            tail = tail->next;
            lista = lista->next;
            tail->next = NULL;
        }
        else if (listb != NULL) {
            tail->next = listb;
            tail = tail->next;
            listb = listb->next;
            tail->next = NULL;
        }
        else {
            break;
        }
    }
    return head.next;
}

NODE *list_merge(NODE *lista, NODE *listb) {
    NODE *list = list_merge_bare(lista->next, listb->next);
    lista->next = list;
    free (listb);
    return lista;
}

NODE *list_eg1(NODE *list) {
    /* 排除空链表、一个元素、两个元素的情况 */
    if (list->next == NULL || list->next->next == NULL || 
        list->next->next->next == NULL) {
        return list;
    }
    /* 找中间靠左节点 */
    NODE *l=list->next, *r=list->next;
    while (r != NULL) {
        r = r->next;
        if (r != NULL) {
            r = r->next;
            if (r != NULL) {
                l = l->next;
            }
        }
    }
    /* 切分为两个链表，后一个链表没有头节点 */
    r=l->next;
    l->next = NULL;
    /* 逆置后一个链表，使用原地反向法 */
    l = r;
    NODE *r1=NULL,*r2=NULL,*r3=NULL; // 链表上连续的三个节点
    r1 = r;
    r2 = r->next;
    if (r2 != NULL) 
        r3 = r2->next;
    while (r2 != NULL) {
        r2->next = r1;
        r1 = r2;
        r2 = r3;
        if (r3) {
            r3 = r3->next;
        }
    }
    l->next = NULL;
    /* 合并两个链表 */
    l = list->next; // 第一个链表的头节点
    r = r1; // 第二个链表的头节点
    NODE *last = list; // 合并后链表的尾节点
    while (1) {
        NODE *ll = NULL,*rr = NULL;
        if (l) {
            ll = l->next;
            last->next = l;
            last = last->next;
            last->next = NULL;
            l = ll;
        }
        if (r) {
            rr = r->next;
            last->next = r;
            last = last->next;
            last->next = NULL;
            r = rr;
        }
        if (l == NULL && r == NULL) {
            break;
        }
    }
    return list;
}