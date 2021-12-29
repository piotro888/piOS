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
    new_node->val = val;
    
    if(list->first == NULL) {
        list->first = new_node;
        list->last = new_node;
    } else {
        list->last->next = new_node;
        list->last = new_node;
    }
}