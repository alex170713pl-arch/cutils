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
    const char* name;
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
GCTable* curr;
GCTable* gtable;

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