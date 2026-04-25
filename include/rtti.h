#ifndef AURORA_RTTI_H
#define AURORA_RTTI_H
#include "type.h"
#include <stddef.h>

typedef struct rtti rtti_t;

void rtti_begin(void);
void rtti_end(void);

rtti_t* rtti_new(int type_id);
rtti_t* rtti_new_custom(const char* name);

void rtti_register(const char* name, size_t size);
void rtti_unregister(const char* name);

int  rtti_typeid(const char* name, const rtti_t* obj);
const char* rtti_typeof(const rtti_t* obj);

void rtti_set(rtti_t* r, int type, void* val);
void rtti_cast(rtti_t* r, int type_id);
const void* rtti_ptr(const rtti_t* r);
size_t rtti_sizeof(const char* name,rtti_t* obj);
void rtti_free(rtti_t** r);

#endif
