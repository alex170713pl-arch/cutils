#include "../include/gc.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef struct {
    void* data;
    int lifetime;
    int pinned;
} GCObject;
typedef struct {
    GCObject* tabl;
    size_t tabs;
    size_t tabln;
    const char* name;
} GCTable;
typedef struct {
    GCTable* Tabs;
    size_t c;
    size_t l;
} GCTables;
GCTables tables;
size_t currt = (size_t)-1;
size_t gtable = (size_t)-1;
char* __strdup(const char* input) {
    if (!input) return NULL;
    size_t i = strlen(input) + 1;
    char* news = malloc(i);
    if (!news) return NULL;
    memcpy(news,input,i);
    return news;
}
size_t _get_tab(void) {
    if (!tables.Tabs) {
        tables.Tabs = calloc(12, sizeof(GCTable));
        if (!tables.Tabs) return (size_t)-1;
        tables.c = 12;
        tables.l = 0;
    }
    if (tables.c == tables.l) {
        size_t newc = tables.c * 2;
        GCTable* new_tabs = realloc(tables.Tabs, newc * sizeof(GCTable));
        if (!new_tabs) return (size_t)-1;
        tables.Tabs = new_tabs;
        tables.c = newc;
    }
    return tables.l++; // индекс
}

GCObject* _get_obj(GCTable* t) {
    if (!t) return NULL;
    if (!t->tabl) {
        t->tabl = calloc(12, sizeof(GCObject));
        if (!t->tabl) return NULL;
        t->tabs = 12;
        t->tabln = 0;
    }
    if (t->tabs == t->tabln) {
        size_t newl = t->tabs * 2;
        GCObject* objs = realloc(t->tabl, newl * sizeof(GCObject));
        if (!objs) return NULL;
        t->tabl = objs;
        t->tabs = newl;
    }
    return &t->tabl[t->tabln++];
}
size_t _find_p(GCTable* t , void* p) {
    if (!t || !p) return (size_t)-1;
    size_t i;
    for (i = 0 ; i < t->tabln;i++)
        if (t->tabl[i].data == p)
            return i;
    return (size_t)-1;
}
void gc_select(void* p, ...) {
    if (gtable == (size_t)-1) {
        gtable = _get_tab();
        if (gtable == (size_t)-1) return;
    }
    if (currt == (size_t)-1) {
        currt = _get_tab();
        if (currt == (size_t)-1) return;
    }

    GCTable* ct = &tables.Tabs[currt];

    va_list list;
    va_start(list, p);
    void* arg = p;
    while (arg) {
        GCObject* o = _get_obj(ct);
        if (!o) { va_end(list); return; }
        o->data = arg;
        o->lifetime = 1;
        o->pinned = 0;
        arg = va_arg(list, void*);
    }
    va_end(list);
}
void gc_set_life_time(void* selected,int time) {
    if (time <= 0) return;
    GCTable* T = &tables.Tabs[currt];
    size_t i = _find_p(T,selected);
    if (i == (size_t)-1) return;
    T->tabl[i].lifetime = time;
}
void gc_lock(void* p) {
    GCTable* T = &tables.Tabs[currt];
    size_t i = _find_p(T,p);
    if (i == (size_t)-1) return;
    T->tabl[i].pinned = 1;
}
void gc_unlock(void* p) {
    GCTable* T = &tables.Tabs[currt];
    size_t i = _find_p(T,p);
    if (i == (size_t)-1) return;
    T->tabl[i].pinned = 0;
}
void gc_collect(int collections_count) {
    if (collections_count <= 0) return;

    GCTable* T = &tables.Tabs[currt];
    size_t i;
    for ( i = 0; i < T->tabln; ) {
        GCObject* c = &T->tabl[i];
        if (!c->pinned) {
            c->lifetime -= collections_count;
            if (c->lifetime <= 0) {
                free(c->data);
                T->tabln--;
                if (i < T->tabln)
                    T->tabl[i] = T->tabl[T->tabln];
                continue;
            }
        }
        i++;
    }
}
void gc_table(const char* table_name) {
    if (!table_name) return;
    size_t i = _get_tab();
    if (i == (size_t)-1) return;
    GCTable* T = &tables.Tabs[i];
    if (T->name)
        free(T->name);
    T->name = __strdup(table_name);
}
void gc_setTable(const char* table_name) {
    if (!table_name) return;
    size_t i;
    for (i = 0;i < tables.l;i++) {
        if (tables.Tabs[i].name != NULL &&
            strcmp(tables.Tabs[i].name,table_name) == 0) {
                currt = i;
                break;
            }
    }
}