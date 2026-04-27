#include "../include/vfs.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define GREEN_CLR "\033[32m"
#define BLUE_CLR "\033[34m"
#define NO_CLR "\033[0m"
typedef enum {
    CODE_OK = 0,
    ERROR_FAIL_FREE_FS,
    ERROR_FAIL_FREE_FILE,
    ERROR_FAIL_FREE_DIR,
    ERROR_ACCESS_VIOLATION,
    ERROR_REAL_FILE_NOT_EXIST,
    ERROR_VIRTUAL_FILE_NOT_EXIST,
    ERROR_VIRTUAL_DIR_NOT_EXIST,
    ERROR_VIRTUAL_FS_INVALID,
    ERROR_FAIL_MAKE_DIR,
    ERROR_FAIL_MAKE_FILE,
    ERROR_NAME_INVALID,
    ERROR_ALREADY_EXIST,
    ERROR_DATA_IS_NULL,
    ERROR_FAIL_WRITE_DATA,
    ERROR_UNKNOWN_MODE,
    ERROR_FAIL_READ_DATA,
    ERROR_BUFFER_INVALID
} error_code_t;
struct vdir {
    char * name;

    vfile_t * files;
    size_t len_f;
    size_t max_f;

    vdir_t * dirs;
    size_t len_d;
    size_t max_d;
    vdir_t * par;
};
struct vfile  {
    char * name;
    void * data;
    vdir_t * parent;
    size_t data_size;
    size_t data_max;
    vfile_open_modes mode;
    void * current_pos;
};
struct vfs {
    vdir_t start_root;
    vdir_t* curr_root;
};
char * __strdup(const char * s) {
    if (!s) return NULL;
    size_t need_to_copy = strlen(s) + 1;
    char * news = malloc(need_to_copy);
    if (!news) return NULL;
    memcpy(news,s,need_to_copy);
    return news;
}
int __init_dirs(vdir_t * dir) {
    if (!dir) return 0;
    dir->dirs = calloc(10,sizeof(vdir_t));
    if (!dir->dirs) return 0;
    dir->len_d = 0;
    dir->max_d = 10;
    size_t i;
    for (i = 0; i < dir->max_d; i++) {
        dir->dirs[i].par = dir;
    }
    return 1;
} 
vdir_t * __get_dir(vdir_t * dir) {
    if (!dir || !dir->dirs) return NULL;
    if (!dir->dirs)
        if (!__init_dirs(dir)) return NULL;
    if (dir->max_d == dir->len_d) {
        size_t new_max = dir->max_d * 2;
        vdir_t * dirs = realloc(dir->dirs,new_max * sizeof(vdir_t));
        if (!dirs) return NULL;
        dir->dirs = dirs;
        dir->max_d = new_max;
    }
    vdir_t * dir_to_return = &dir->dirs[dir->len_d];
    dir->len_d++;
    return dir_to_return;
}
vfile_t* __get_file(vdir_t * dir) {
    if (!dir) return NULL;
    if (!dir -> files) {
        dir->files = calloc(10,sizeof(vfile_t));
        if (!dir->files) return NULL;
        dir->max_f = 10;
        dir->len_f = 0;
        return &dir->files[dir->len_f];
    }
    if (dir->max_f == dir->len_f) {
        size_t new_max = dir->max_f * 2;
        vfile_t * files = realloc(dir->files,new_max * sizeof(vfile_t));
        if (!files) return NULL;
        dir->files = files;
        dir->max_f = new_max;
    }
    dir->len_f++;
    memset(dir->files,0,dir->max_f * sizeof(vfile_t));
    return &dir->files[dir->len_f - 1];
}
vdir_t* __change_dir(vdir_t* curr, const char* name) {
    size_t i;
    if (!curr || !name) return NULL;
    if (strcmp(name, ".") == 0)
        return curr;
    if (strcmp(name, "..") == 0)
        return curr->par; 

    for (i = 0; i < curr->len_d; i++) {
        if (curr->dirs[i].name &&
            strcmp(curr->dirs[i].name, name) == 0) {
            return &curr->dirs[i];
        }
    }

    return NULL;
}
int __split_parent_last(const char* path, char** parent, char** last) {
    size_t len = strlen(path);
    if (len == 0) return 0;
    const char* p = path + len;
    while (p != path && *p != '/') p--;
    if (*p != '/') return 0;
    *last = __strdup(p + 1);
    if (!*last) return 0;
    size_t plen = p - path;
    if (plen == 0) {
        *parent = __strdup("/");
        if (!*parent) {
            free(*last);
            return 0;
        }
        return 1;
    }
    *parent = malloc(plen + 1);
    if (!*parent) {
        free(*last);
        return 0;
    }
    memcpy(*parent, path, plen);
    (*parent)[plen] = '\0';
    return 1;
}
