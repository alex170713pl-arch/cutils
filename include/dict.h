#ifndef AURORA_DICT_H
#define AURORA_DICT_H

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif
typedef struct dict dict_t;
#ifndef __cplusplus
    #include <stddef.h>
#endif
dict_t* dict_new(void);
void dict_push(dict_t* d, const char* str_key, const void* data, size_t size_of_data);
void* dict_get(dict_t* d, const char* str_key, size_t* out_s);
int dict_delete(dict_t* d, const char* str_key);
void dict_find(dict_t* d, const char* str_key, size_t out[2]);
void dict_foreach(dict_t* d, void (*fptr)(const char* key, void* value));
void dict_free(dict_t** d);
#define DICT_PUSH(d,k,v,t) \
    do { \
        t __temp = v;\
        dict_push(d,k,&__temp,sizeof(t));\
    } while(0)
#ifdef __cplusplus
}
#endif

#endif
