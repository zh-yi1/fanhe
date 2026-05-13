#ifndef __LIST_H
#define __LIST_H

// 链表节点结构体
struct Node {
    uint32_t *ptr;
    struct Node* next;
};

// 在链表尾部插入节点
void insertNode(struct Node** head, uint32_t *value);

// 删除链表中第一个匹配的节点
void deleteNode(struct Node** head, uint32_t *value);

// 在链表中查询节点
struct Node* searchNode(struct Node* head, uint32_t *value);

// 打印链表
void printList(struct Node* head);

// 释放链表内存
void freeList(struct Node** head);

#endif // __LIST_H
