#ifndef AURORA_GC
    #define AURORA_GC
    #include <stddef.h>
    #define FREE_OLD 1
    #define NOT_FREE_OLD 0
    void gc_select(void* p,...);
    void gc_set_life_time(void* selected,int time);
    void gc_lock(void* p);
    void gc_unlock(void* p);
    void gc_collect(int collections_count);
    void gc_table(const char* table_name);
    void gc_setTable(const char* table_name);
    void gc_resetTable(int free_old);
    void* gc_allocate(size_t size,int flags);
    void gc_unselect(void* p,...);
    void gc_export(void* p,const char* exportObjName);
    void* gc_import(const char* importObjName);
    #define GC_BLOCK(name, b)            \
        do {                             \
            gc_table(name);              \
            gc_setTable(name);           \
            b                            \
            gc_resetTable(1);            \
        } while (0)

    void gc_status();
    void gc_end(void);
#endif