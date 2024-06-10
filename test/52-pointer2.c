#include <stdio.h>

struct foo {
    int val;
    struct foo *next;
};

struct foo head, mid, tail;

int main() {
    struct foo *ptr;
    head.val = 10;
    head.next = &mid;
    mid.val = 15;
    mid.next = &tail;
    tail.val = 20;
    tail.next = NULL;

    ptr = &head;
    printf("### head = %d %d\n", head.val, ptr->val);
    printf("mid = %d %d\n", mid.val, ptr->next->val);
    printf("tail = %d %d\n", tail.val, ptr->next->next->val);
    return (0);
}