#include "../include/cutils-shared_ptr.h"
#include <stdlib.h>
struct shared {
    void* ptr;
    size_t refs;
};
struct weak {
    struct shared* __targ__;
};
shared_ptr* shared_new(size_t size) {
    shared_ptr* new = malloc(sizeof(shared_ptr));
    if (!new) return NULL;
    new->ptr = malloc(size);
    if (!new->ptr) {
        free(new);
        return NULL;
    }
    new->refs = 1;
    return new;
}
shared_ptr* shared_copy(shared_ptr* targ) {
    if (!targ || !targ->ptr) return NULL;
        targ->refs++;
    return targ;
}
void* shared_get(shared_ptr* targ) {
    if (!targ || !targ->ptr) return NULL;
    return targ->ptr;
}
size_t shared_getrefs(shared_ptr* targ){
    if (!targ || !targ->ptr) return 0;
    return targ->refs;
}
int shared_isvalid(shared_ptr* p) {
    return  p && p->ptr;
}
void shared_realloc(shared_ptr* p,size_t __newsize) {
    if (!shared_isvalid(p)) return;
    if (__newsize == 0 ) return;
    void* new = realloc(p->ptr,__newsize);
    if (!new) return;
    p->ptr = new;
}
void shared_free(shared_ptr** targ) {
    if (!targ || !*targ) return;
    
    shared_ptr* p = *targ;
    p->refs--;
    if (p->refs > 0) {
        *targ = NULL;
        return;
    }
    free(p->ptr);
    free(p);
    *targ = NULL;
}
weak_ptr* weak_new(shared_ptr* targ){
    weak_ptr* new = calloc(1,sizeof(weak_ptr));
    new->__targ__ = targ;
    return new;
}
void weak_change(weak_ptr* targ,shared_ptr* new) {
    if (!targ) return;
    targ->__targ__ = new;
}
shared_ptr* weak_lock(weak_ptr* __ptr) {
    if (
        !__ptr 
        || 
        !__ptr->__targ__ 
        || 
        !__ptr->__targ__->ptr
    ) return NULL;

    shared_ptr* new = __ptr->__targ__;
    if (!new->ptr) return NULL;
    new->refs++;
    return new;
}
void weak_free(weak_ptr** __ptr) {
    if (!*__ptr) return;
    free(*__ptr);
    *__ptr = NULL;
}