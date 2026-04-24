#ifndef ONE_OWN
    #define ONE_OWN
    #include <stddef.h>
    typedef struct one_owner one_ownerptr;

    one_ownerptr* one_owner_create(size_t size);
    one_ownerptr* one_owner_move(one_ownerptr* __dest__);
    void* one_owner_get(one_ownerptr* p);
    void one_owner_realloc(one_ownerptr* __ptr,size_t __newSize);
    int one_owner_isvalid(one_ownerptr* p);
    #ifndef one_owner_set
        #define one_owner_set(p,t,v) \
        do {\
            if (one_owner_isvalid(p)) {\
                *(t*)one_owner_get(p) = v; \
            } \
        } while(0)
    #endif
    void one_owner_free(one_ownerptr** p);
#endif