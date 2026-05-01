#include "../include/rtti.h"
#include "../include/obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define ERROR_CONTEXT_FATAL 1
#define ERROR_CONTEXT_STD 0
typedef struct {
    obj_class_t ** classes;
    size_t cap;
    size_t len;
} __global_list;
static __global_list list = {0};
void push_err(const char* error_msg,int context) {

    if (!error_msg) {
        fputs("\033[95m",stderr);
        fputs("[Aurora-obj] : FATAL : Error Message is NULL!",stderr);
        fputs("\033[0m\n",stderr);
        goto ABORT;
    }
    if (context == ERROR_CONTEXT_FATAL) {
        fputs("\033[95m",stderr);
        fputs(error_msg,stderr);
        fputs("\033[0m\n",stderr);
        goto ABORT;
    }
    fputs("\033[91m",stderr);
    fputs(error_msg,stderr);
    fputs("\033[0m\n",stderr);
    #ifndef AURORA_OBJ_SOFT_ERRORS
        goto ABORT;
    #endif
    return;
    ABORT:
        UNWIND_ALL();
        abort(); 
}
void __global_list_add(__global_list * s,obj_class_t* c) {
    if (!s) return;
    if (!c) {
        #ifndef AURORA_OBJ_SOFT_ERRORS
            abort();
        #endif
        return;
    }

    if (!s->classes) {
        s->classes = calloc(10,sizeof(obj_class_t*));
        if (!s->classes) {
        #ifndef AURORA_OBJ_SOFT_ERRORS 
            abort(); 
        #endif
            return;
        }
        s->cap = 10;
        s->len = 0;
    }
    if (s->cap == s->len) {
        size_t new_max = s->cap * 2;
        obj_class_t** new_bf = realloc(s->classes,new_max * sizeof(obj_class_t*));
        if (!new_bf) {
            #ifndef AURORA_OBJ_SOFT_ERRORS 
                abort(); 
            #endif
            return;
        }
        s->classes = new_bf;
        s->cap = new_max;
    }
    s->classes[s->len] = c;
    s->len++;
}
typedef struct {
    char dum;
} __secret;
typedef struct {
    void** in;
    size_t cap;
    size_t len;
    size_t* elem_sizes;
} __container_t;
char * __strdup(const char * d) {
    if (!d) return NULL;
    size_t s = strlen(d) + 1;
    char* ptr = malloc(s);
    if (!ptr) return NULL;
    memcpy(ptr,d,s);
    return ptr;
}
void __container_push(__container_t** c,void* data,size_t datas) {
    if (!(*c)) {
        *c = malloc(sizeof(__container_t));
        if (!(*c)) return;
        (*c)->cap = 0;
        (*c)->in = NULL;
        (*c)->len = 0;
    }
    if (!(*c)->in) {
        (*c)->in = calloc(10,sizeof(void*));
        (*c)->elem_sizes = calloc(10,sizeof(size_t));
        if (!(*c)->in || !(*c)->elem_sizes) return;
        (*c)->cap = 10;
        (*c)->len = 0;
    }
    if ((*c)->cap == (*c)->len) {
        size_t new_s = (*c)->cap * 2;
        void** newb = realloc((*c)->in,new_s * sizeof(void*));
        size_t* new_sizes = realloc((*c)->elem_sizes,new_s * sizeof(size_t));
        if (!newb) return;
        (*c)->in = newb;
        (*c)->cap = new_s;
    }
    (*c)->in[(*c)->len] = calloc(1,datas);
    if (!(*c)->in[(*c)->len]) return;
    (*c)->elem_sizes[(*c)->len] = datas;
    memcpy((*c)->in[(*c)->len],data,datas);
    (*c)->len++;
}
int __container_find(__container_t* c,void* datap,size_t datas) {
    if (!c || !c->in || !datap || c->len == 0) return -1;
    size_t i;
    for (i = 0; i < c->len;i++) {
        if (memcmp(c->in[i],datap,datas) == 0)
            return i;
    }
    return -1;
}
int __container_find_str(__container_t * c , char* data) {
    if (!c || !c->in || !data || c->len == 0) return -1;
    size_t i;
    for (i = 0; i < c->len;i++) {
        if (strcmp(c->in[i],data) == 0)
            return i;
    }
    return -1;
}
void __container_contact(__container_t* targ,__container_t* from) {
    if (!from || !targ) return;
    size_t i;
    for (i = 0;i < from->len;i++)
        __container_push(targ,from->in[i],from->elem_sizes[i]);
    
}
void __container_free(__container_t* c) {
    if (!c || !c->in) return;
    size_t i;
    for (i = 0 ; i < c->len;i++) {
        free(c->in[i]);
    }
    free(c->in);
}
void __container_unique(__container_t** targ) {
    __container_t* new = NULL;
    size_t i;
    for (i = 0; i < (*targ)->len;i++) {
        if (__container_find(new,(*targ)->in[i],(*targ)->elem_sizes[i]) == -1) {
            __container_push(&new,(*targ)->in[i],(*targ)->elem_sizes[i]);
        }
    }
    *targ = new;
}
void __container_unique_str(__container_t** targ) {
    __container_t* new = NULL;
    size_t i;
    for (i = 0; i < (*targ)->len;i++) {
        if (__container_find_str(new,(*targ)->in[i]) == -1) {
            __container_push(&new,(*targ)->in[i],(*targ)->elem_sizes[i]);
        }
    }
    *targ = new;
}
void UNWIND_ALL(void);
struct obj_class {
    char * name;
    obj_constructor _constructor;
    obj_destructor _destructor;

    __container_t* extend_mets_names;
    __container_t* extend_mets_access;
    __container_t* extend_mets;

    __container_t* extend_fields_names;
    __container_t* extend_fields_access;

    __container_t* extend_vtable;
    __container_t* extend_vtable_mets_names;
    __container_t* extend_vtable_accesses;

    unsigned char extend : 1;
    __container_t* objects;

    __container_t* ext_interfaces;
    __container_t* need_to_realisate;
    __container_t* realisated;

    __container_t* methods_name;
    __container_t* methods;
    __container_t* methods_accesses;

    __container_t* fields_name;
    __container_t* fields_accesses;

    __container_t* vtable;
    __container_t* vtable_mets_names;
    __container_t* vtable_accesses;
    unsigned char is_abstr : 1;
};
struct obj {
    obj_class_t * parent;
    __container_t fields;
    __secret s;
};
struct obj_interf {
    __container_t names;
    char* interf_name;
};
obj_class_t* obj_new_class(const char * name,obj_constructor __init__,obj_destructor __del__) {
    rtti_begin();
    if (!name) {
            fputs("[Aurora-obj]: ERROR : creating class : name is NULL\n",stderr);
        #ifndef AURORA_OBJ_SOFT_ERRORS
            UNWIND_ALL();
            abort();
        #endif
        return NULL;
    }
    obj_class_t * c = calloc(1,sizeof(obj_class_t));
    if (!c) {
        fprintf(stderr,"[Aurora-obj] : ERROR : fail to create class %s\n",name);
        #ifndef AURORA_OBJ_SOFT_ERRORS 
            abort(); 
        #endif
        return NULL;
    }
    c->name = __strdup(name);
    if (!c->name) {
        fprintf(stderr,"[Aurora-obj] : ERROR : fail to create class %s\n",name);
        #ifndef AURORA_OBJ_SOFT_ERRORS 
            abort(); 
        #endif
        free(c);
        return NULL;
    }
    c->_constructor = __init__;
    c->_destructor = __del__;
    return c;
}

void obj_class_new_field(obj_class_t * class,int type,const char* name,access_level level) {
    if (!class) {
        push_err("[Aurora-obj] : ERROR : Invalid Class!\n",0);
        return;
    }
    if (!name) {
        fputs("[Aurora-obj] : ERROR : field name must not be null!\n",stderr);
        #ifndef AURORA_OBJ_SOFT_ERRORS
            UNWIND_ALL();
            abort(); 
        #endif
        return;
    }
    rtti_t* __Temp = rtti_new(type);
    if (!__Temp) {
        push_err("[Aurora-obj] : FATAL : Fail Allocate Help Data!",1);
        return;
    }
    if (rtti_typeid(NULL,__Temp) == -1)  {
        push_err("[Aurora-obj] : ERROR: Type ID Invalid!",0);
        return;
    }
    if (level > 3) {
        push_err("[Aurora-obj] : ERROR : Access Level Invalid!",0);
        return;
    }
    __container_push(class->fields_name,name,strlen(name) + 1);
    __container_push(class->fields_accesses,&level,sizeof(level));
}
void obj_class_add_method(obj_class_t * class,obj_met method,const char* method_name,access_level level) {
    if (!class) {
        push_err("[Aurora-obj] : ERROR : Invalid Class!\n",0);
        return;
    }
    if (!method_name) {
        push_err("[Aurora-obj] : ERROR :Invalid name!",0);
        return;
    }
    if (method && class->is_abstr) {
        char to_print[9064];
        sprintf(to_print,"[Aurora-obj] : ERROR : Abstract Class %s can't have a realisation of method %s",class->name,method_name);
        push_err(to_print,0);
        return;
    } else if (!method && !class->is_abstr){
        char to_print[9064];
        sprintf(to_print,"[Aurora-obj] : ERROR :  Class %s can't have a unrealisated method %s",class->name,method_name);
        push_err(to_print,0);
        return;
    }
    if (level > 3) {
        push_err("[Aurora-obj] : ERROR : Access Level Invalid!",0);
        return;
    }
    __container_push(class->methods,&method,sizeof(method));
    __container_push(class->methods_name,method_name,strlen(method_name) + 1);
    __container_push(class->methods_accesses,&level,sizeof(level));
}
int obj_class_extend(obj_class_t * who,obj_class_t * from) {
    if (!who) {
        push_err("[Aurora-obj] : ERROR : Invalid class (who) !\n",0);
        return -1;
    }
    if (!from) {
        push_err("[Aurora-obj] : ERROR : Invalid class (from) !\n",0);
        return -1;
    }
    if (who->extend || from->extend) {
        push_err("[Aurora-obj] : ERROR : Multiple extend forbidden!\n",0);
        return -1;
    }
    size_t i;
    for (i = 0;i < from->fields_name->len;i++) {
        if (*(access_level*)from->fields_accesses->in[i] != ACCESS_PRIVATE) {
            __container_push(&who->extend_fields_names,
                from->fields_name->in[i],
                from->fields_name->elem_sizes[i]
            );
            __container_push(&who->extend_fields_access,
                from->fields_accesses->in[i],
                from->fields_accesses->elem_sizes[i]
            );
        }
    }
    for (i = 0; i < from->methods->len;i++) {
        if (*(access_level*)from->methods_accesses->in[i] != ACCESS_PRIVATE) {
            __container_push(&who->extend_mets,
                from->methods->in[i],
                from->methods->elem_sizes[i]
            );
            __container_push(&who->extend_mets_names,
                from->methods_name->in[i],
                from->methods_name->elem_sizes[i]
            );
            __container_push(&who->extend_mets_access,
                from->fields_accesses->in[i],
                from->fields_accesses->elem_sizes[i]
            );
        }
    }
    for (i = 0 ; i < from->vtable->len;i++) {
        if (*(access_level*)from->vtable_accesses->in[i] != 0) {
            __container_push(&who->extend_vtable,
                from->vtable->in[i],
                from->vtable->elem_sizes[i]
            );
            __container_push(&who->extend_vtable_mets_names,
                from->vtable_mets_names->in[i],
                from->vtable_mets_names->elem_sizes[i]
            );
            __container_push(&who->extend_vtable_accesses,
                from->vtable_accesses->in[i],
                from->vtable_accesses->elem_sizes[i]
            );
        }
    }
}