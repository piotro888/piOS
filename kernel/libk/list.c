#include "list.h"
#include <libk/kmalloc.h>
#include <libk/types.h>

void list_init(struct list* list) {
    list->first = NULL;
    list->last = NULL;
}

void list_append(struct list* list, void* val) {
    struct list_node* new_node = kmalloc(sizeof(struct list_node));
    new_node->next = NULL;
    new_node->prev = list->last;
    new_node->val = val;
    
    if(list->first == NULL) {
        list->first = new_node;
        list->last = new_node;
    } else {
        list->last->next = new_node;
        list->last = new_node;
    }
}

void list_remove(struct list* list, struct list_node* list_node) {
    if(list_node->next)
        list_node->next->prev = list_node->prev;
    else
        list->last = list_node->prev;

    if(list_node->prev)
        list_node->prev->next = list_node->next;
    else
        list->first = list_node->next;

    kfree(list_node);
}
