#ifndef LIBK_LIST_H
#define LIBK_LIST_H

struct list_node {
    struct list_node* next;
    struct list_node* prev;
    void* val;
};

struct list {
    struct list_node* first;
    struct list_node* last;
};

void list_init(struct list* list);

void list_append(struct list* list, void* val);
void list_remove(struct list* list, struct list_node* list_node);

#define list_foreach(list) for(struct list_node* it=(list)->first; it != NULL; it = it->next)
#define LIST_FOREACH_VAL(type) ((type) it->val)
#define LIST_FOREACH_NODE (it)

#endif
