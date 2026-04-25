/* aurora-signal.c */
#include "../include/signal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct handle {
    worker_sig worker;
    char* trigger_msg;
    size_t runs;
    size_t runs_limit;
    signal_t* registered;
};

struct signal_group {
    signal_t** signals;
    size_t len;
    size_t max;
};

struct signal {
    handle_t** handles;
    size_t len;
    size_t max;
    struct signal* chain_next_node;
    char* trigger_msg;
};

typedef struct {
    signal_t** signals;
    size_t len;
    size_t max;
} __global_signals_list;

static __global_signals_list list = {NULL, 0, 0};

static char* __strdup(const char* t) {
    char* c;
    size_t s;
    
    if (!t) return NULL;
    s = strlen(t) + 1;
    c = malloc(s);
    if (!c) return NULL;
    memcpy(c, t, s);
    return c;
}

static void __register_sig(signal_t* news) {
    if (!list.signals) {
        list.signals = calloc(256, sizeof(signal_t*));
        if (!list.signals) return;
        list.max = 256;
    }
    if (list.len >= list.max) {
        size_t new_max = list.max * 2;
        signal_t** new_list;
        
        new_list = realloc(list.signals, new_max * sizeof(signal_t*));
        if (!new_list) return;
        list.max = new_max;
        list.signals = new_list;
    }
    list.signals[list.len] = news;
    list.len++;
}

static void __unregister_sig(signal_t* sig) {
    size_t i;
    
    if (!list.signals || !sig) return;
    
    if (list.len == 0) {
        free(list.signals);
        list.signals = NULL;
        list.max = 0;
        return;
    }
    
    for (i = 0; i < list.len; i++) {
        if (list.signals[i] == sig) {
            list.signals[i] = list.signals[list.len - 1];
            list.len--;
            return;
        }
    }
}

static void __signal_del_handle(struct signal* s, struct handle* h) {
    size_t i;
    
    if (!s || !s->handles || !h) return;
    
    for (i = 0; i < s->len; i++) {
        if (s->handles[i] == h) {
            if (i < s->len - 1) {
                memmove(&s->handles[i], &s->handles[i + 1],
                        (s->len - i - 1) * sizeof(struct handle*));
            }
            s->len--;
            return;
        }
    }
}

signal_t* signal_new(void) {
    struct signal* new_sig;
    
    new_sig = calloc(1, sizeof(struct signal));
    if (!new_sig) return NULL;
    
    new_sig->handles = calloc(32, sizeof(struct handle*));
    if (!new_sig->handles) {
        free(new_sig);
        return NULL;
    }
    
    new_sig->max = 32;
    __register_sig(new_sig);
    return new_sig;
}

handle_t* handle_new(worker_sig fn) {
    struct handle* new_handle;
    
    if (!fn) return NULL;
    
    new_handle = calloc(1, sizeof(struct handle));
    if (!new_handle) return NULL;
    
    new_handle->worker = fn;
    new_handle->runs_limit = (size_t)-1;  /* бесконечность */
    return new_handle;
}

void handle_runOn(handle_t* h, const char* targs) {
    if (!h) return;
    
    if (h->trigger_msg) {
        free(h->trigger_msg);
        h->trigger_msg = NULL;
    }
    
    if (targs) {
        h->trigger_msg = __strdup(targs);
    }
}

size_t handle_runs(handle_t* h) {
    if (!h) return 0;
    return h->runs;
}

void signal_chain(signal_t* target, signal_t* node) {
    signal_t* last;
    if (!target || !node) return;
    last = target;
    while (last->chain_next_node)
        last = last->chain_next_node;
    last->chain_next_node = node;
}

void handle_setMaxRuns(handle_t* h, size_t runs) {
    if (!h) return;
    h->runs_limit = runs;
    h->runs = 0;
}

void handle_free(handle_t** h) {
    if (!h || !*h) return;
    
    if ((*h)->registered) {
        __signal_del_handle((*h)->registered, *h);
    }
    
    if ((*h)->trigger_msg) {
        free((*h)->trigger_msg);
    }
    
    free(*h);
    *h = NULL;
}

void signal_connect(signal_t* s, handle_t* fn) {
    if (!s || !s->handles || !fn || !fn->worker) return;
    
    if (s->len >= s->max) {
        size_t new_max;
        handle_t** new_handles;
        
        new_max = s->max * 2;
        new_handles = realloc(s->handles, new_max * sizeof(handle_t*));
        if (!new_handles) return;
        
        s->max = new_max;
        s->handles = new_handles;
    }
    
    fn->registered = s;
    s->handles[s->len] = fn;
    s->len++;
}

void signal_emit(signal_t* s, const char* message, void** data, void* ret) {
    size_t i;
    
    if (!s || !s->handles || !message) return;
    
emit:
    for (i = 0; i < s->len; i++) {
        handle_t* h = s->handles[i];
        int matches;
        
        if (!h || !h->worker) continue;
        
        matches = (h->trigger_msg == NULL) ||
                  (strcmp(h->trigger_msg, message) == 0);
        
        if (matches && h->runs < h->runs_limit) {
            h->worker(data, ret);
            h->runs++;
        }
    }
    
    if (s->chain_next_node) {
        s = s->chain_next_node;
        goto emit;
    }
}

void signal_free(signal_t** s) {
    size_t i;
    
    if (!s || !*s || !(*s)->handles) return;
    
    for (i = 0; i < (*s)->len; i++) {
        handle_t* h = (*s)->handles[i];
        handle_free(&h);
    }
    
    if ((*s)->trigger_msg) {
        free((*s)->trigger_msg);
    }
    
    free((*s)->handles);
    free(*s);
    *s = NULL;
}

void signal_shared_emit(const char* message, void** d, void* r) {
    size_t i;
    
    for (i = 0; i < list.len; i++) {
        if (list.signals[i]) {
            signal_emit(list.signals[i], message, d, r);
        }
    }
}

size_t signal_handles(signal_t* s) {
    if (!s) return 0;
    return s->len;
}

size_t signal_total_signals(void) {
    return list.len;
}

signal_group_t* signal_new_group(void) {
    signal_group_t* g;
    
    g = malloc(sizeof(signal_group_t));
    if (!g) return NULL;
    
    g->signals = malloc(sizeof(signal_t*) * 32);
    if (!g->signals) {
        free(g);
        return NULL;
    }
    
    g->len = 0;
    g->max = 32;
    return g;
}

void signal_group_emit(signal_group_t* g, const char* msg, void** data, void* ret) {
    size_t i;
    
    if (!g || !g->signals || !msg) return;
    
    for (i = 0; i < g->len; i++) {
        if (g->signals[i]) {
            if (g->signals[i]->trigger_msg == NULL ||
                strcmp(g->signals[i]->trigger_msg, msg) == 0) {
                signal_emit(g->signals[i], msg, data, ret);
            }
        }
    }
}

void signal_set_trigger_message(signal_t* s, const char* msg) {
    if (!s) return;
    
    if (s->trigger_msg) {
        free(s->trigger_msg);
        s->trigger_msg = NULL;
    }
    
    if (msg) {
        s->trigger_msg = __strdup(msg);
    }
}

void signal_connect_to_group(signal_group_t* g, signal_t* s) {
    if (!g || !g->signals || !s) return;
    
    if (g->len >= g->max) {
        size_t new_max;
        signal_t** new_buff;
        
        new_max = g->max * 2;
        new_buff = realloc(g->signals, new_max * sizeof(signal_t*));
        if (!new_buff) return;
        
        g->max = new_max;
        g->signals = new_buff;
    }
    
    g->signals[g->len] = s;
    g->len++;
}

void signal_disconnect_from_group(signal_group_t* g, signal_t* s) {
    size_t i;
    
    if (!g || !g->signals || !s || g->len == 0) return;
    
    for (i = 0; i < g->len; i++) {
        if (g->signals[i] == s) {
            if (i < g->len - 1) {
                memmove(&g->signals[i], &g->signals[i + 1],
                        (g->len - i - 1) * sizeof(signal_t*));
            }
            g->len--;
            return;
        }
    }
}
void signal_group_free(signal_group_t** g) {
    if (!g || !*g) return;

    if ((*g)->signals) {
        free((*g)->signals);
    }
    
    free(*g);
    *g = NULL;
}

#ifndef AURORA_DISABLE_DEBUG_FUNCTIONS
void handle_dump(handle_t* h) {
    if (!h) {
        printf("===== HANDLE IS NULL =====\n");
        return;
    }
    
    printf("===== DUMP OF HANDLE %p =====\n", (void*)h);
    printf("    WORKER: %p\n", (void*)h->worker);
    printf("    RUNS: %lu / %lu (0 = inf)\n",
           (unsigned long)h->runs, (unsigned long)h->runs_limit);
    printf("    TRIGGER MESSAGE: %s\n",
           h->trigger_msg ? h->trigger_msg : "(null)");
    printf("    REGISTERED IN SIGNAL: %p\n", (void*)h->registered);
    printf("===== END DUMP OF HANDLE %p =====\n", (void*)h);
}

void signal_dump(signal_t* s) {
    size_t i;
    
    if (!s) {
        printf("===== SIGNAL IS NULL =====\n");
        return;
    }
    
    printf("===== DUMP OF SIGNAL: %p =====\n", (void*)s);
    printf("    HANDLERS: %lu\n", (unsigned long)s->len);
    printf("    TRIGGER MESSAGE: %s\n",
           s->trigger_msg ? s->trigger_msg : "(null)");
    printf("    NEXT IN CHAIN: %p\n", (void*)s->chain_next_node);
    
    if (s->len > 0) {
        printf("    HANDLERS:\n");
        for (i = 0; i < s->len; i++) {
            handle_t* h = s->handles[i];
            printf("        [%lu] HANDLE: %p\n",
                   (unsigned long)i, (void*)h);
            printf("            RUNS: %lu / %lu (0 = inf)\n",
                   (unsigned long)h->runs, (unsigned long)h->runs_limit);
            printf("            TRIGGER MSG: %s\n",
                   h->trigger_msg ? h->trigger_msg : "(null)");
        }
    }
    
    printf("===== END OF DUMP SIGNAL: %p =====\n", (void*)s);
}
#endif