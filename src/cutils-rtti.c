#include "../include/cutils-rtti.h"
#include <stdlib.h>
#include <string.h>
typedef struct {
    int* ids;
    char** types;
    size_t* sizes;
} control_block;
struct rtti {
    int mag_num;
    void* data;
};
static const size_t builtin_sizes[] = {
    0,          

    sizeof(int), 
    sizeof(short), 
    sizeof(long), 
    sizeof(char), 
    sizeof(float), 
    sizeof(double), 
    sizeof(long double),
    
    sizeof(unsigned int),
    sizeof(unsigned short),
    sizeof(unsigned long),
    sizeof(unsigned char),

    sizeof(int*),        
    sizeof(short*),        
    sizeof(long*),        
    sizeof(char*),        
    sizeof(float*),        
    sizeof(double*),        
    sizeof(long double*),  
    sizeof(void*),        

    sizeof(unsigned int*),
    sizeof(unsigned short*),
    sizeof(unsigned long*),
    sizeof(unsigned char*) 
};
static const char* builtin_names[] = {
    NULL,               

    "int",       
    "short",     
    "long",      
    "char",      
    "float",     
    "double",    
    "long double",

    "unsigned int",     
    "unsigned short",   
    "unsigned long",    
    "unsigned char",    

    "int*",             
    "short*",           
    "long*",            
    "char*",            
    "float*",           
    "double*",          
    "long double*",     
    "void*",            

    "unsigned int*",    
    "unsigned short*",  
    "unsigned long*",   
    "unsigned char*"    
};
static size_t types_len;
static control_block global = {NULL,NULL,NULL};
char* __strdup(const char* __str) {
    if (!__str) return NULL;
    size_t ln = strlen(__str);
    char* heap_str = calloc(ln + 1,sizeof(char));
    if (!heap_str) return NULL;
    memcpy(heap_str,__str,ln + 1);
    return heap_str;
}
int __append_str(const char* __str) {
    if (!__str) return -1;
    if (!global.types) return -1;

    char** new_strs = realloc(global.types, (types_len + 1) * sizeof(char*));
    if (!new_strs) return -1;

    new_strs[types_len] = __strdup(__str);
    if (!new_strs[types_len]) return -1;

    global.types = new_strs;
    return 0;
}

int __append_size(size_t size) {
    size_t* new_sizes = realloc(global.sizes, (types_len + 1) * sizeof(size_t));
    if (!new_sizes) return -1;

    new_sizes[types_len] = size;
    global.sizes = new_sizes;
    return 0;
}

int __append_id(void) {
    int* new_ids = realloc(global.ids, (types_len + 1) * sizeof(int));
    if (!new_ids) return -1;

    new_ids[types_len] = (int)types_len;
    global.ids = new_ids;
    return 0;
}

int __find_id(int data) {
    if (!global.ids) 
        return -1;

    size_t count = types_len;
    size_t i;

    for (i = 0;i < count;i++){
        if (global.ids[i] == data)
            return i;
    }
    return -1;
}
int __find_str(const char* type_name) {
    if (!type_name) return -1;
    if (!global.types) return -1;
    size_t i;

    for (i = 1; i < types_len;i++) {
        if (strcmp(type_name,global.types[i]) == 0)
            return i;
    }

    return -1;
}
void init_block(control_block* bk) {
    size_t count = sizeof(builtin_sizes) / sizeof(builtin_sizes[0]); 
    types_len = count;
    bk->ids   = calloc(count, sizeof(int));
    bk->sizes = calloc(count, sizeof(size_t));
    bk->types = calloc(count, sizeof(char*));

    if (!bk->ids || !bk->sizes || !bk->types) {
        free(bk->ids);
        free(bk->sizes);
        free(bk->types);
        bk->ids = NULL;
        bk->sizes = NULL;
        bk->types = NULL;
        return;
    }

    size_t i;
    for ( i = 1; i < count; i++) {
        bk->ids[i]   = i;
        bk->sizes[i] = builtin_sizes[i];
        bk->types[i] = __strdup(builtin_names[i]); 
    }
    
}
void rtti_begin(void) {
    if (global.ids == NULL) {
        init_block(&global);
    }
}
void rtti_end(void) {
    if (global.ids) {
        free(global.ids);
        global.ids = NULL;
        free(global.sizes);
        global.sizes = NULL;
        size_t i;
        for (i = 0; i < types_len;i++) {
            free(global.types[i]);
        }
        
        free(global.types);
        global.types = NULL;
    }
}
rtti_t* rtti_new(int type_id) {
    if (__find_id(type_id) == -1) 
        return NULL;
    rtti_t* new_rtti = calloc(1,sizeof(rtti_t));
    if (!new_rtti) return NULL;
    new_rtti->mag_num = type_id;
    new_rtti->data = NULL;
    return new_rtti;
}
rtti_t* rtti_new_custom(const char* name) {
    int ind = __find_str(name);
    if (ind == -1) return NULL;
    return rtti_new(ind);
}

void rtti_register(const char* name, size_t size) {
    if (!name) return;

    if (__append_str(name) == -1) return;
    if (__append_size(size) == -1) return;
    if (__append_id()   == -1) return;

    types_len++;
}
void rtti_unregister(const char* name) {
    if (!name) return;
    if (!global.ids || !global.sizes || !global.types) return;
    int ind = __find_str(name);
    if (ind == -1) return;
    int need_move = types_len - (ind + 1);
    free(global.types[ind]);
    if (need_move > 0) {
        memmove(&global.types[ind],
                &global.types[ind + 1],
                need_move * sizeof(char*));

        memmove(&global.sizes[ind],
                &global.sizes[ind + 1],
                need_move * sizeof(size_t));

        memmove(&global.ids[ind],
                &global.ids[ind + 1],
                need_move * sizeof(int));
    }

    types_len--;
}
int  rtti_typeid(const char* name, const rtti_t* obj) {
    if ((name && obj) ||(!name && !obj)) return -1;
    if (!name && obj)
        return obj->mag_num;
    else 
        return __find_str(name);
}
const char* rtti_typeof(const rtti_t* obj) {
    if (!obj) return NULL;
    if (!global.types) return NULL;

    int id = obj->mag_num;
    if (id < 0 || id >= types_len)
        return NULL;

    return global.types[id];
}
void rtti_set(rtti_t* r, int type, void* val) {
    if (!r || !r->data || !val) return;
    if (type >= types_len  || type < 0) return;

    int s  = global.sizes[type];
    if (s != global.sizes[r->mag_num]) return;
    memcpy(r->data,val,s);
}
void rtti_cast(rtti_t* r, int type_id) {
    if (!r || !r->data) return;
    if (type_id < 0 || type_id >= types_len) return;

    size_t new_size = global.sizes[type_id];
    size_t old_size = global.sizes[r->mag_num];
    void* new_data = malloc(new_size);
    if (!new_data) return;

    size_t copy_size = old_size < new_size ? old_size : new_size;
    memcpy(new_data, r->data, copy_size);

    free(r->data);
    r->data = new_data;
    r->mag_num = type_id;
}
const void* rtti_ptr(const rtti_t* r) {
    if (!r || !r->data) return NULL;
    return r->data;
}
void rtti_free(rtti_t** r) {
    if (!r || !(*r) ) return;
    free((*r)->data);
    free(*r);
    *r = NULL;
}