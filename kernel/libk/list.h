#ifndef LIBK_LIST_H
#define LIBK_LIST_H

struct list_node {
    struct list_node* next;
    void* val;
};

struct list {
    struct list_node* first;
    struct list_node* last;
};

void list_init(struct list* list);

void list_append(struct list* list, void* val);

#define list_foreach(list) for(struct list_node* it=(list)->first; it != NULL; it = it->next)
#define LIST_FOREACH_VAL(type) (type) it->val

#endif