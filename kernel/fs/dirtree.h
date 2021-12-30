#ifndef FS_DIRTREE_H
#define FS_DIRTREE_H

#include <libk/list.h>
#include <fs/tar.h>

/*
 * Structure for holding directory strucure and file lookup in filesystems.
 */


struct dir_t_node {
    char name[100];
    struct dir_t_node* parent;

    struct list subdirs; // list of dir_t_node
    struct list files; // list of file_t
};

void dir_tree_init(struct dir_t_node* root);

void dir_tree_add_path(struct dir_t_node* root, struct file_t* file);

struct file_t* dir_tree_get_file(struct dir_t_node* node, char* path);

void dir_tree_printf(struct dir_t_node* node, int depth);

#endif