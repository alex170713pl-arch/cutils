#include "../include/dict.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#ifndef SIZE_MAX
    #define SIZE_MAX ((size_t)-1)
#endif
typedef unsigned long u32;
typedef struct {
    u32 hash;
    void* data;
    size_t size;
    char* key;
} obj;
typedef struct {
    obj* objects;
    size_t cap;
    size_t len;
} bucket;
struct dict {
    bucket* arr;
    size_t len;
    size_t cap;
};

u32 djb(const char* input) {
    if (!input) return 0UL;
    u32 hsh = 5381;
    u32 c;
    while((c = *input++))
        hsh = ((hsh << 5 ) + hsh) ^ c;
    return hsh;
}
static char* __strdup(const char* input) {
    if (!input) return NULL;
    size_t len = strlen(input) + 1;
    char* new = malloc(len);
    if (!new) return NULL;
    memcpy(new,input,len);
    return new;
}
int obj_cmp_by_hash(const void* a, const void* b) {
    const obj* A = a;
    const obj* B = b;
    if (A->hash < B->hash) return -1;
    if (A->hash > B->hash) return 1;
    return 0;
}

static void bucket_binary_insert(bucket* b, obj o) {
    if (b->len == b->cap) {
        b->cap = b->cap ? b->cap * 2 : 8;
        b->objects = realloc(b->objects, b->cap * sizeof(obj));
        if (!b->objects) return;
    }
    size_t lo = 0;
    size_t hi = b->len;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (b->objects[mid].hash < o.hash)
            lo = mid + 1;
        else
            hi = mid;
    }
    memmove(&b->objects[lo + 1],
            &b->objects[lo],
            (b->len - lo) * sizeof(obj));
    b->objects[lo] = o;
    b->len++;
}
void dict_rehash(dict_t* d) {
    size_t old_cap = d->cap;
    size_t new_cap = old_cap * 2;
    size_t i, j;
    bucket* new_arr = calloc(new_cap, sizeof(bucket));
    if (!new_arr) return;
    for (i = 0; i < old_cap; i++) {
        bucket* b = &d->arr[i];
        for (j = 0; j < b->len; j++) {
            obj o = b->objects[j];
            size_t idx = o.hash % new_cap;
            bucket* nb = &new_arr[idx];
            bucket_binary_insert(nb, o);
        }
        free(b->objects);
    }
    free(d->arr);
    d->arr = new_arr;
    d->cap = new_cap;
}

int _push_to_buck(bucket* b, const char* key,const void* data, size_t s) {
    if (!b || !data || !s || !key) return 0;
    if (!b->cap) {
        b->objects = calloc(25, sizeof(obj));
        if (!b->objects) return 0;
        b->cap = 25;
        b->len = 0;
    }
    u32 hash = djb(key);
    if (!hash) return 0;
    obj o;
    o.key = __strdup(key);
    if (!o.key) return 0;
    o.data = malloc(s);
    if (!o.data) {
        free(o.key);
        return 0;
    }
    memcpy(o.data, data, s);
    o.hash = hash;
    o.size = s;
    bucket_binary_insert(b, o);
    return 1;
}
void bdelete(bucket* b, size_t idx) {
    if (!b || idx >= b->len)
        return;
    free(b->objects[idx].data);
    if (idx < b->len - 1) {
        memmove(
            &b->objects[idx],
            &b->objects[idx + 1],
            (b->len - idx - 1) * sizeof(obj)
        );
    }
    b->len--;
}
dict_t* dict_new(void) {
    dict_t* d = calloc(1,sizeof(dict_t));
    if (!d) return NULL;
    return d;
}
void dict_push(dict_t* d, const char* str_key, const void* data, size_t size_of_data) {
    if (!d || !str_key || !data || !size_of_data) return;
    u32 hash = djb(str_key);
    if (!hash) return;
    if (!d->cap) {
        d->arr = calloc(25, sizeof(bucket));
        if (!d->arr) return;
        d->cap = 25;
        d->len = 0;
    }
    if ((d->len / (float)d->cap) >= 0.75f)
        dict_rehash(d);
    size_t idx = hash % d->cap;
    if (_push_to_buck(&d->arr[idx], str_key, data, size_of_data))
        d->len++;
}
void dict_find(dict_t* d, const char* str_key, size_t out[2]) {
    out[0] = SIZE_MAX;
    out[1] = SIZE_MAX;
    if (!d || !str_key || !d->cap)
        return;
    u32 h = djb(str_key);
    if (!h)
        return;
    size_t buck = h % d->cap;
    bucket* b = &d->arr[buck];
    if (b->len == 0)
        return;
    obj key = { .hash = h };
    obj* found = bsearch(
        &key,
        b->objects,
        b->len,
        sizeof(obj),
        obj_cmp_by_hash
    );
    if (!found)
        return;
    out[0] = buck;
    out[1] = found - b->objects;
}
void* dict_get(dict_t* d,const char* str_key,size_t*out_s) {
    if (!d || !d->arr || !str_key || !out_s) return NULL;
    size_t indexes[2];
    dict_find(d,str_key,indexes);
    if (indexes[0] == SIZE_MAX || indexes[1] == SIZE_MAX) return NULL;
    obj* o = &d->arr[indexes[0]].objects[indexes[1]];
    if (!o->data) return NULL;
    void* newd = malloc(o->size);
    if (!newd) return NULL;
    memcpy(newd,o->data,o->size);
    *out_s = o->size;
    return newd;
}
int dict_delete(dict_t* d,const char* str_key) {
    if (!d || !str_key) return 0;
    size_t o[2];
    dict_find(d,str_key,o);
    if (o[0] == SIZE_MAX || o[1] == SIZE_MAX) return 0;
    bdelete(&d->arr[o[0]],o[1]);
    d->len--;
    return 1;
}
void dict_foreach(dict_t* d,void(fptr)(const char* key,void* value)) {
    if (!d || !fptr) return;
    size_t i,j;
    for (i = 0;i < d->cap;i++) 
        for (j = 0; j < d->arr[i].len;j++)
            fptr(d->arr[i].objects[j].key,d->arr[i].objects[j].data);
}
void dict_free(dict_t** d) {
    if (!d || !*d) return;

    dict_t* dc = *d;
    size_t i,j;
    for ( i = 0; i < dc->cap; i++) {
        bucket* b = &dc->arr[i];
        for ( j = 0; j < b->len; j++) {
            obj* o = &b->objects[j];
            free(o->key);
            free(o->data);
        }
        free(b->objects);
    }
    free(dc->arr);
    free(dc);
    *d = NULL;
}
