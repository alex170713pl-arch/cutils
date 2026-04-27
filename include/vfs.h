#ifndef AURORA_VFS  
    #define AURORA_VFS
    #include <stddef.h>

    typedef struct vfs vfs_t;
    typedef struct vdir vdir_t;
    typedef struct vfile vfile_t;
    typedef enum {
        MODE_READ,
        MODE_WRITE,
        MODE_APPEND,
        MODE_WRITE_READ,
        MODE_READ_WRITE,
        MODE_APPEND_READ
    } vfile_open_modes;
    vfs_t * vfs_new(void);
    vdir_t * vfs_open_dir(vfs_t * fs,const char* path,const char* dir_name);
    vfile_t * vfile_open(vfs_t * fs, const char* path, vfile_open_modes mode);
    int vfile_write(vfile_t * f,void* data,size_t ln);
    int vfile_read(vfile_t * f,void* buff,size_t len);
    const char * vfs_err_msg(int code);
    int vfile_copy_from_real(vfile_t * f,const char* real_file_path);
    void vdir_files(vdir_t * dir);
    void vdir_dirs(vdir_t * dir);
    void vdir_list(vdir_t * dir);
    int vfile_close(vfile_t** f);
    int vdir_close(vdir_t * d);
    int vfs_free(vfs_t** fs);
    int vfile_rewind(vfile_t* file);
    int vfs_move(vfs_t * fs,const char * path,const char * file_name,const char * target_path);
    int vfs_remove(vfs_t * fs,vdir_t * par,const char* file_name);
    int vfs_rename(vfs_t * fs,const char * path,const char* file_name,const char * new_file_name);
    int vfs_chroot(vfs_t * fs,const char* new_root);
    int vfs_exit_from_chroot(vfs_t * fs);
    int vfs_move_dir(vfs_t * fs,const char * dir_path,const char * new_dir_path);
    int vfs_remove_dir(vdir_t *parent, const char *name);
    int vfs_rename_dir(vfs_t * fs,const char * dir_path,const char * new_dir_name);
    int vfs_mkdir(vfs_t * fs,const char * path,const char* dir_name);
    int vfs_exist_file(vfs_t* fs,const char * file_path);
    int vfs_exist_dir(vfs_t* fs,const char * file_path);
#endif