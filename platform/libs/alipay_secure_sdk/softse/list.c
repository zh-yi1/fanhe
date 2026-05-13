#include "include.h"
#include "list.h"
#if SECURITY_PAY_EN

// 在链表尾部插入节点
void insertNode(struct Node** head, uint32_t *value) {
    // 创建新节点
    //struct Node* newNode = (struct Node*)func_zalloc(sizeof(struct Node));
    //struct Node* newNode = (struct Node*)ab_malloc(sizeof(struct Node));
    struct Node* newNode = (struct Node*)SECURITY_MALLOC(sizeof(struct Node));

    newNode->ptr = value;
    newNode->next = NULL;

    // 如果链表为空，将新节点作为头节点
    if (*head == NULL) {
        *head = newNode;
//        printf("%s head\n", __func__);
        return;
    }

    // 找到链表尾部节点
    struct Node* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
//    printf("### %s: newNode: 0x%x, value: 0x%x\n", __func__, newNode, value);
    // 将新节点连接到链表尾部
    current->next = newNode;
//    printf("%s: end\n", __func__);

}

// 删除链表中第一个匹配的节点
void deleteNode(struct Node** head, uint32_t *value) {
    // 如果链表为空，直接返回
    if (*head == NULL) {
        return;
    }

    // 如果要删除的节点是头节点
    if ((*head)->ptr == value) {
        struct Node* temp = *head;
        *head = (*head)->next;
//        printf("delete point: 0x%x\n", temp->ptr);
//        printf("delete point: 0x%x\n", temp);
        //func_free(temp->ptr);
        //func_free(temp);

        //ab_free(temp->ptr);
        //ab_free(temp);
        SECURITY_FREE(temp->ptr);
        SECURITY_FREE(temp);
        return;
    }

    // 找到要删除节点的前一个节点
    struct Node* current = *head;
    while (current->next != NULL && current->next->ptr != value) {
        current = current->next;
    }

    // 如果找到了要删除的节点
    if (current->next != NULL) {
        struct Node* temp = current->next;
        current->next = current->next->next;
//        printf("delete point: 0x%x\n", temp->ptr);
//        printf("delete point: 0x%x\n", temp);
        //func_free(temp->ptr);
        //func_free(temp);

        //ab_free(temp->ptr);
        //ab_free(temp);

        SECURITY_FREE(temp->ptr);
        SECURITY_FREE(temp);
    }
}

// 在链表中查询节点
struct Node* searchNode(struct Node* head, uint32_t *value) {
    // 如果链表为空，返回NULL
    if (head == NULL) {
        return NULL;
    }

    // 找到具有特定值的节点
    struct Node* current = head;
    while (current != NULL && current->ptr != value) {
        current = current->next;
    }

    return current;
}

// 打印链表
void printList(struct Node* head) {
    printf("%s\n", __func__);
    struct Node* current = head;
    while (current != NULL) {
        printf("0x%x\n", current->ptr);
        current = current->next;
    }
}

// 释放链表内存
void freeList(struct Node** head) {
//    printf("%s\n", __func__);
    struct Node* current = *head;
    while (current != NULL) {
        struct Node* temp = current;
        current = current->next;
//         printf("free: 0x%x\n", temp->ptr);
//         printf("node free: 0x%x\n", temp);
        //func_free(temp->ptr);
        //func_free(temp);

        //ab_free(temp->ptr);
        //ab_free(temp);
        SECURITY_FREE(temp->ptr);
        SECURITY_FREE(temp);
    }
    *head = NULL;  // 将头指针置为空
}
#endif // SECURITY_PAY_EN
