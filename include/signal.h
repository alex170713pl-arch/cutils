#ifndef AURORA_SIGNAL
    #include <stddef.h>

    typedef struct signal signal_t;
    typedef struct handle handle_t;
    typedef struct signal_group signal_group_t;
    typedef void(*worker_sig)(void**,void*);
    signal_t* signal_new(void);
    void signal_chain(signal_t* target,signal_t* node);
    void signal_connect(signal_t* s,handle_t* fn);
    void signal_emit(signal_t* s, const char* message,void** data,void* ret);
    void signal_free(signal_t** s);
    void signal_shared_emit(const char* message,void** d,void* r);
    handle_t* handle_new(worker_sig fn);
    void handle_runOn(handle_t* h,const char* targs);
    size_t handle_runs(handle_t* h);
    void handle_setMaxRuns(handle_t* h, size_t runs);
    void handle_free(handle_t** h);

    size_t signal_handles(signal_t* s);
    size_t signal_total_signals();

    signal_group_t* signal_new_group();
    void signal_group_emit(signal_group_t* g,const char* msg,void** data,void* ret);
    void signal_set_trigger_message(signal_t* s, const char* msg);
    void signal_connect_to_group(signal_group_t * g,signal_t* s);
    void signal_disconnect_from_group(signal_group_t* g,signal_t* s);
    void signal_group_free(signal_group_t** g);

    #ifndef AURORA_DISABLE_DEBUG_FUNCTIONS
        void signal_dump(signal_t* s);
        void handle_dump(handle_t* h);
    #endif
#endif