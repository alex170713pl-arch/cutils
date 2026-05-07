#include "../include/gc.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
    void* data;
    int lifetime;
    int pinned;
} GCObject;

typedef struct _Block {
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
    unsigned char import_state;
} Import_GCObject;

typedef struct _Ibk {
    struct _Ibk* next;
    Import_GCObject* o;
    size_t in;
    size_t cap;
} Import_GCBlock;

typedef struct {
    Import_GCBlock* header;
} Import_Table;

#ifndef FREE_OLD
#define FREE_OLD 1
#endif

Import_Table tab;
GCTables tables;
GCTable* curr = NULL;
GCTable* gtable = NULL;
GCTable* last;
static char* __strdup(const char* input) {
    if (!input) return NULL;
    size_t i = strlen(input) + 1;
    char* news = malloc(i);
    if (!news) return NULL;
    memcpy(news, input, i);
    return news;
}

GCTable* _get_table(void) {
    if (!tables.Tabs) {
        tables.Tabs = calloc(12, sizeof(GCTable*));
        if (!tables.Tabs) return NULL;
        tables.l = 0;
        tables.c = 12;
    }
    GCTable* new = calloc(1, sizeof(GCTable));
    if (!new) return NULL;
    if (tables.c == tables.l) {
        size_t newc = tables.c * 2;
        GCTable** newb = realloc(tables.Tabs, newc * sizeof(GCTable*));
        if (!newb) return NULL;
        tables.Tabs = newb;
        tables.c = newc;
    }
    tables.Tabs[tables.l] = new;
    tables.l++;
    return new;
}

GCBlock* _get_block(GCTable* table) {
    if (!table) return NULL;
    if (!table->header) {
        GCBlock* b = calloc(1, sizeof(GCBlock));
        if (!b) return NULL;
        table->header = b;
        return b;
    }
    GCBlock* cur = table->header;
    GCBlock* last = NULL;
    while (cur) {
        if (cur->obj == NULL)
            return cur;
        if (cur->in < cur->max)
            return cur;
        last = cur;
        cur = cur->next;
    }
    GCBlock* newb = calloc(1, sizeof(GCBlock));
    if (!newb) return NULL;
    if (last) last->next = newb;
    return newb;
}

GCObject* _get_object(GCBlock* bk) {
    if (!bk) return NULL;
    if (!bk->obj) {
        bk->obj = calloc(128, sizeof(GCObject));
        if (!bk->obj) return NULL;
        bk->max = 128;
        bk->in = 0;
        return &bk->obj[bk->in++];
    }
    if (bk->in >= bk->max)
        return NULL;
    return &bk->obj[bk->in++];
}

Import_GCBlock* _get_imp_block(Import_Table* table) {
    if (!table) return NULL;
    if (!table->header) {
        Import_GCBlock* bk = calloc(1, sizeof(Import_GCBlock));
        if (!bk) return NULL;
        table->header = bk;
        return bk;
    }
    Import_GCBlock* bk = table->header;
    Import_GCBlock* lst = NULL;
    while (bk) {
        if (bk->o == NULL)
            return bk;
        if (bk->in < bk->cap)
            return bk;
        lst = bk;
        bk = bk->next;
    }
    Import_GCBlock* b = calloc(1, sizeof(Import_GCBlock));
    if (!b) return NULL;
    if (lst) lst->next = b;
    return b;
}

Import_GCObject* _get_imp_object(Import_GCBlock* bk) {
    if (!bk) return NULL;
    if (!bk->o) {
        bk->o = calloc(128, sizeof(Import_GCObject));
        if (!bk->o) return NULL;
        bk->cap = 128;
        bk->in = 0;
    }
    if (bk->in < bk->cap) {
        return &bk->o[bk->in++];
    }
    if (bk->next) {
        return _get_imp_object(bk->next);
    }
    return NULL;
}

int _find_p(GCBlock* bk, void* p) {
    if (!bk || !bk->obj) return -1;
    size_t i;
    for (i = 0; i < bk->in; i++)
        if (bk->obj[i].data == p)
            return 1;
    return 0;
}

int _find_p_ind(GCBlock* bk, void* p) {
    if (!bk || !bk->obj) return -1;
    size_t i;
    for (i = 0; i < bk->in; i++)
        if (bk->obj[i].data == p)
            return (int)i;
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

int _imported(const char* name) {
    if (!name) return -1;
    Import_GCBlock* o = tab.header;
    while (o) {
        size_t i;
        for (i = 0; i < o->in; i++) {
            if (strcmp(o->o[i].name, name) == 0)
                return 1;
        }
        o = o->next;
    }
    return 0;
}

void gc_select(void* p, ...) {
    if (!gtable && !curr) {
        gtable = _get_table();
        if (!gtable) return;
        curr = gtable;
    }
    if (!curr && gtable) curr = gtable;
    if (!curr) return;

    va_list list;
    va_start(list, p);
    void* arg = p;
    while (arg) {
        GCBlock* bk = _get_block(curr);
        if (!bk) break;
        int rs = _find_p(bk, arg);
        if (rs == -1 || rs == 0) {
            GCObject* o = _get_object(bk);
            if (!o) {
                bk = bk->next ? bk->next : _get_block(curr);
                if (!bk) break;
                o = _get_object(bk);
                if (!o) break;
            }
            o->data = arg;
            o->lifetime = 1;
            o->pinned = 0;
        }
        arg = va_arg(list, void*);
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
    size_t i;
    for (i = 0; i < (size_t)inds[0]; i++) {
        if (!bk) return;
        bk = bk->next;
    }
    if (!bk) return;
    GCObject* o = &bk->obj[inds[1]];
    o->pinned = 1;
}

void gc_unlock(void* p) {
    if (!p || !curr) return;
    int inds[2];
    _find_in_t(curr, p, inds);
    if (inds[0] == -1 || inds[1] == -1) return;
    GCBlock* b = curr->header;
    size_t i;
    for (i = 0; i < (size_t)inds[0]; i++) {
        if (!b) return;
        b = b->next;
    }
    if (!b) return;
    GCObject* o = &b->obj[inds[1]];
    o->pinned = 0;
}

void gc_collect(int collections_count) {
    if (collections_count <= 0) return;
    if (!curr) return;
    GCBlock* b = curr->header;
    while (b) {
        if (b->obj) {
            size_t in;
            for (in = 0; in < b->in; in++) {
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
    for (i = 0; i < tables.l; i++) {
        if (tables.Tabs[i]->name && strcmp(tables.Tabs[i]->name, table_name) == 0) {
            last = curr;
            curr = tables.Tabs[i];
            return;
        }
    }
}

void gc_resetTable(void) {
    if (!curr)
        return;
    GCBlock* b = curr->header;
    while (b) {
        GCBlock* next = b->next;
        size_t i;
        if (b->obj) {
            for ( i = 0; i < b->in; i++)
                free(b->obj[i].data);
            free(b->obj);
        }

        free(b);
        b = next;
    }
    curr->header = NULL;
    curr = last;
}

void* gc_allocate(size_t size) {
    void* p = malloc(size);
    if (!p) return NULL;
    gc_select(p, NULL);
    return p;
}

void gc_unselect(void* p, ...) {
    if (!p || !curr) return;
    va_list list;
    void* arg = p;
    va_start(list, p);
    while (arg) {
        int pos[2];
        _find_in_t(curr, arg, pos);
        if (pos[0] >= 0 && pos[1] >= 0) {
            GCBlock* blk = curr->header;
            int i = pos[0];
            while (i-- && blk)
                blk = blk->next;
            if (blk && (size_t)pos[1] < blk->in) {
                GCObject* o = &blk->obj[(size_t)pos[1]];
                o->data = NULL;
                o->lifetime = 0;
                o->pinned = 0;
            }
        }
        arg = va_arg(list, void*);
    }
    va_end(list);
}

void gc_export(void* p, const char* exportObjName) {
    if (!p || !exportObjName) return;
    if (_imported(exportObjName)) return;
    int out[2] = {0, 0};
    _find_in_t(curr, p, out);
    if (out[0] == -1 || out[1] == -1) return;

    GCBlock* b = curr->header;
    size_t i;
    for (i = 0; i < (size_t)out[0]; i++)
        b = b->next;
    if (!b) return;

    Import_GCBlock* ib = _get_imp_block(&tab);
    if (!ib) return;
    Import_GCObject* io = _get_imp_object(ib);
    if (!io) return;

    io->import_state = 1;
    io->name = __strdup(exportObjName);
    if (!io->name) {
        io->import_state = 0;
        if (ib->in > 0) ib->in--;
        return;
    }
    io->o = &b->obj[out[1]];
    io->o->pinned = 1;
}

void* gc_import(const char* importObjName) {
    if (!importObjName) return NULL;
    Import_GCBlock* b = tab.header;
    while (b) {
        size_t i;
        for (i = 0; i < b->in; i++) {
            if (b->o[i].name && strcmp(b->o[i].name, importObjName) == 0) {
                if (b->o[i].o) {
                    b->o[i].o->pinned = 0;
                    return b->o[i].o->data;
                }
            }
        }
        b = b->next;
    }
    return NULL;
}

GCStatus_t* gc_status(void) {
    GCStatus_t* stat = malloc(sizeof(GCStatus_t));
    if (!stat) return NULL;
    memset(stat, 0, sizeof(GCStatus_t));
    stat->tables = tables.l;
    size_t ti;
    for ( ti = 0; ti < tables.l; ti++) {
        GCTable* t = tables.Tabs[ti];
        if (!t) continue;
        GCBlock* b = t->header;
        while (b) {
            stat->blocks++;
            if (b->obj && b->in > 0) {
                size_t oi;
                for ( oi = 0; oi < b->in; oi++) {
                    GCObject* o = &b->obj[oi];
                    stat->total_memory += sizeof(GCObject);
                    if (o->data) {
                        stat->objects++;
                        stat->used_memory += sizeof(GCObject);
                        if (o->pinned) stat->pinned++;
                    } else {
                        stat->free_objects++;
                    }
                }
            }
            b = b->next;
        }
    }

    Import_GCBlock* ib = tab.header;
    while (ib) {
        stat->imported += ib->in;
        ib = ib->next;
    }
    return stat;
}

void gc_begin(void) {
    if (!tables.Tabs) {
        tables.Tabs = calloc(12, sizeof(GCTable*));
        if (!tables.Tabs) return;
        tables.c = 12;
        tables.l = 0;
    }
    if (!gtable) {
        gtable = _get_table();
        if (gtable) curr = gtable;
    }
}

void gc_end(void) {
    size_t i;
    for (i = 0; i < tables.l; i++) {
        GCTable* t = tables.Tabs[i];
        if (!t) continue;

        curr = t;
        gc_resetTable();
        free(t->name);
        free(t);
    }
    free(tables.Tabs);
    tables.Tabs = NULL;
    tables.l = tables.c = 0;
    curr = NULL;
    gtable = NULL;
        Import_GCBlock *ib = tab.header;
    while (ib) {
        Import_GCBlock *next = ib->next;
        if (ib->o) {
            for (i = 0; i < ib->in; i++) {
                free(ib->o[i].name);
            }
            free(ib->o);
        }
        free(ib);
        ib = next;
    }
    tab.header = NULL;
}
