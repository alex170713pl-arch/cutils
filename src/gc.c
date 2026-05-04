#include "../include/gc.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef struct {
    void* data;
    int lifetime;
    int pinned;
} GCObject;
typedef struct _Block{
    GCObject* obj;
    size_t max;
    size_t in;
    struct _Block* next;
} GCBlock;
typedef struct {
    GCBlock* header;
    char* name;
} GCTable;
typedef struct {
    GCTable** Tabs;
    size_t c;
    size_t l;
} GCTables;

typedef struct {
    char* name;
    GCObject* o;
} Import_GCObject;
typedef struct _Ibk{
    struct _Ibk* next;
    Import_GCObject* o;
    size_t in;
    size_t cap;
} Import_GCBlock;
typedef struct {
    Import_GCBlock* header;
} Import_Table;

Import_Table tab;
GCTables tables;
GCTable* curr = NULL;
GCTable* gtable = NULL;

char* __strdup(const char* input) {
    if (!input) return NULL;
    size_t i = strlen(input) + 1;
    char* news = malloc(i);
    if (!news) return NULL;
    memcpy(news,input,i);
    return news;
}
GCTable* _get_table(void) {
    if (!tables.Tabs) {
        tables.Tabs = calloc(12,sizeof(GCTable*));
        if (!tables.Tabs) return  NULL;
        tables.l = 0;
        tables.c = 12;
    }
    GCTable* new = calloc(1,sizeof(GCTable));
    if (!new) return  NULL;
    if (tables.c == tables.l) {
        size_t newc = tables.c * 2;
        GCTable** newb = realloc(tables.Tabs,newc * sizeof(GCTable*));
        if (!newb) return NULL;
        tables.Tabs = newb;
        tables.c = newc;
    }
    tables.Tabs[tables.l] = new;
    tables.l++;
    return  new;
}
GCBlock* _get_block(GCTable* table) {
    if (!table) return NULL;
    if (!table->header) {
        GCBlock* b = calloc(1, sizeof(GCBlock));
        if (!b) return NULL;
        table->header = b;
        return b;
    }
    GCBlock* curr = table->header;
    GCBlock* last = NULL;
    while (curr) {
        if (curr->obj == NULL)
            return curr;
        if (curr->in < curr->max)
            return curr;
        last = curr;
        curr = curr->next;
    }
    GCBlock* newb = calloc(1, sizeof(GCBlock));
    if (!newb) return NULL;
    last->next = newb;
    return newb;
}
GCObject* _get_object(GCBlock* bk) {
    if (!bk) return NULL;
    if (!bk->obj) {
        bk->obj = calloc(128, sizeof(GCObject));
        if (!bk->obj) return NULL;
        bk->max = 128;
        bk->in  = 0; 
        return &bk->obj[bk->in++];
    }
    if (bk->in >= bk->max)
        return NULL;
    return &bk->obj[bk->in++];
}
int _find_p(GCBlock* bk,void* p) {
    if (!bk || !bk->obj) return -1;
    int i;
    for (i = 0;i < bk->in;i++)
        if (bk->obj[i].data == p)
            return 1;
    return 0;
}
int _find_p_ind(GCBlock* bk,void* p) {
    if (!bk || !bk->obj) return -1;
    int i;
    for (i = 0; i < bk->in;i++)
        if (bk->obj[i].data == p)
            return i;
    return -1;
}
void _find_in_t(GCTable* t, void* p, int out[2]) {
    out[0] = -1;
    out[1] = -1;
    if (!t || !p) return;
    GCBlock* bk = t->header;
    int b = 0;
    while (bk) {
        int idx = _find_p_ind(bk, p);
        if (idx != -1) {
            out[0] = b;
            out[1] = idx;
            return;
        }
        bk = bk->next;
        b++;
    }
}

void gc_select(void* p,...) {
    if (!gtable && !curr) {
        gtable = _get_table();
        if (!gtable) return;
        curr = gtable;
    }
    if (!curr && gtable) curr = gtable;
    GCBlock* bk = _get_block(curr);
    if (!bk) return;
    va_list list;
    va_start(list,p);
    void* arg = p;
    while (arg) {
        int rs = _find_p(bk,arg);
        if (rs != -1 && !rs) {
            GCObject* o = _get_object(bk);
            if (!o) { 
                bk = (bk->next) ? bk->next : _get_block(curr);
                if (!bk) {va_end(list); return;}
                o = _get_object(bk);
                if (!o) { va_end(list); return; }
            }
            o->data = arg;
            o->lifetime = 1;
            o->pinned = 0;
        }
        arg = va_arg(list,void*);
    }
    va_end(list);
}
void gc_set_life_time(void* selected, int time) {
    if (!selected || !curr || !gtable || time <= 0) return;
    GCTable* t = curr;
    GCBlock* bk = t->header;
    if (!bk) return;
    while (bk) {
        int idx = _find_p_ind(bk, selected);
        if (idx != -1) {
            bk->obj[idx].lifetime = time;
            return;
        }
        bk = bk->next;
    }
}
void gc_lock(void* p) {
    if (!p || !curr) return;
    int inds[2];
    _find_in_t(curr, p, inds);
    if (inds[0] == -1 || inds[1] == -1) return;
    GCBlock* bk = curr->header;
    int i;
    for ( i = 0; i < inds[0]; i++) {
        if (!bk) return;
        bk = bk->next;
    }
    GCObject* o = &bk->obj[inds[1]];
    o->pinned = 1;
}
void gc_unlock(void* p) {
    if (!p || !curr) return;
    int inds[2];
    _find_in_t(curr, p,inds);
    if (inds[0] == -1 || inds[1] == -1) return;
    GCBlock* b = curr->header;
    int i;
    for (i = 0; i < inds[0];i++) {
        if (!b) return;
        b = b->next;
    }
    GCObject* o = &b->obj[inds[1]];
    o->pinned = 0;
}
void gc_collect(int collections_count) {
    if (collections_count <= 0) return;
    GCBlock* b = curr->header;
    while (b ) {
        if (b->obj) {
            size_t in;
            for (in = 0 ; in < b->in;in++) {
                GCObject* o = &b->obj[in];
                if (!o->pinned && o->data)
                    o->lifetime -= collections_count;
                if (o->lifetime <= 0 && o->data) {
                    free(o->data);
                    o->data = NULL;
                    o->lifetime = 0;
                }
            }
        }
        b = b->next;
    }
}
void gc_table(const char* table_name) {
    if (!table_name) return;
    GCTable* newt = _get_table();
    if (!newt) return;
    newt->name = __strdup(table_name);
    if (!newt->name) {
        free(newt);
        return;
    }
}
void gc_setTable(const char* table_name) {
    if (!table_name) return;
    size_t i;
    for (i = 0; i < tables.l;i++) {
        if (tables.Tabs[i]->name && strcmp(tables.Tabs[i]->name,table_name) == 0)
            curr = tables.Tabs[i];
    }
}
void gc_resetTable(int free_old) {
    if (free_old && curr) {
        GCBlock* b = curr->header;
        while (b) {
            GCBlock* next = b->next;
            for (size_t i = 0; i < b->in; i++) {
                free(b->obj[i].data);
            }
            free(b->obj);
            free(b);
            b = next;
        }
        free(curr->name);
        free(curr);
    }
    curr = gtable;
}
void* gc_allocate(size_t size) {
    void* p = malloc(size);
    if (!p) return NULL;
    gc_select(p,NULL);
    return p;
}