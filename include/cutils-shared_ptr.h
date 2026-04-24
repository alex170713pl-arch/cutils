#ifndef SHARED
    #define SHARE
    #include <stddef.h>
    typedef struct shared shared_ptr;
    typedef struct weak weak_ptr;
    shared_ptr* shared_new(size_t size);
    shared_ptr* shared_copy(shared_ptr* targ);
    void* shared_get(shared_ptr* targ);
    #ifndef shared_set
        #define shared_set(p,t,v) do {\
            if (shared_isvalid(p)) {\
                *(t*)shared_get(p) = v;\
            }\
        }while(0)
    #endif
    size_t shared_getrefs(shared_ptr* targ);
    int shared_isvalid(shared_ptr* p);
    void shared_free(shared_ptr** targ);

    weak_ptr* weak_new(shared_ptr* targ);
    void weak_change(weak_ptr* targ,shared_ptr* new);
    shared_ptr* weak_lock(weak_ptr* __ptr);
    void weak_free(weak_ptr** __ptr);
    
#endif 