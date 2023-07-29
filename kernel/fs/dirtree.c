#include "dirtree.h"
#include <string.h>
#include <libk/assert.h>
#include <libk/list.h>
#include <libk/kprintf.h>
#include <libk/kmalloc.h>
#include <fs/tar.h>

int get_slash_pos(char* s, int n) {
    size_t pos = 0;
    while(*s) {
        if(*s == '/') {
            if(n-- == 0)
                return pos;
        }
        s++;
        pos++;
    }
    return -1;
}

void dir_tree_init(struct dir_t_node* root) {
    strcpy(root->name, "/");
    root->parent = root;
    list_init(&root->subdirs);
    list_init(&root->files);
}

void dir_tree_add_path(struct dir_t_node* root, struct file_t* file) {
    char* path = file->name;

    char* next_slash = path;
    struct dir_t_node* node = root;
    while((next_slash = strchr(next_slash+1, '/')) != NULL) {
        int found = 0;
        list_foreach(&node->subdirs) {
            struct dir_t_node* val = LIST_FOREACH_VAL(struct dir_t_node*);
            if(!strncmp(val->name, path, (next_slash-path)+1)) {
                node = val;
                found = 1;
                break;
            }
        }
        if(found)
            continue;

        struct dir_t_node* new_node = kmalloc(sizeof(struct dir_t_node));
        
        size_t prefix_len = (next_slash-path)+1;
        strncpy(new_node->name, path, prefix_len);
        new_node->name[prefix_len+1] = '\0';

        new_node->parent = node;
        list_init(&new_node->subdirs);
        /* So correct addresses :
            new_node
            &new_node->subdirs
        */
        list_init(&new_node->files);

        list_append(&node->subdirs, new_node);
        node = new_node;
    }

    if(file->name[strlen(file->name)-1] == '/')
        return;

    list_append(&node->files, file);
}

#define INP for(int x=0; x<depth; x++) kprintf(" ");
void dir_tree_printf(struct dir_t_node* node, int depth) {

    INP; kprintf("d: %s\n", node->name);
    depth++;
    list_foreach(&node->files) {
        struct file_t* val = LIST_FOREACH_VAL(struct file_t*);
        INP; kprintf("f: %s\n", val->name);
    }

    list_foreach(&node->subdirs) {
        struct dir_t_node* val = LIST_FOREACH_VAL(struct dir_t_node*);
        dir_tree_printf(val, depth);
    }
}

struct file_t* dir_tree_get_file(struct dir_t_node* node, char* path) {
    int dir_depth = 0, path_index;

    ASSERT(*path == '/');
    path++;
    while((path_index = get_slash_pos(path, dir_depth++)) > 0) {
        int found = 0;
        list_foreach(&node->subdirs) {
            struct dir_t_node* val = LIST_FOREACH_VAL(struct dir_t_node*);

            if(!strncmp(val->name, path, path_index+1)) {
                node = val;
                found = 1;
                break;
            }
        }
        
        if(!found)
            return NULL;
    }

    list_foreach(&node->files) {
        struct file_t* file = LIST_FOREACH_VAL(struct file_t*);
        if(!strcmp(file->name, path)) 
            return file;
    }

    return NULL;
}
