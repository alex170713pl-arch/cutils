# Aurora : light of ANSI C
 **size** : 27KB (.a) , 31KB (.so) | **ANSI C**
## modules

- rtti
- smartptr (one_owner and shared_ptr)
- str
- signal
### API
- rtti
```c
 // allocate memory for rtti 
 void rtti_begin(void);
 // free memory of rtti
 void rtti_end(void);
 //create new rtti object
 rtti_t* rtti_new(int type_id);
 // create new rtti object by name
 rtti_t* rtti_new_custom(const char* name);
 // register new type in rtti 
 void rtti_register(const char* name, size_t size);
 // unregister type in rtti
 void rtti_unregister(const char* name);
 // return type id (by name and by obj)
 int  rtti_typeid(const char* name, const rtti_t* obj); // NULL,obj or "your_type",NULL
 // return type in object
 const char* rtti_typeof(const rtti_t* obj);
 // set value in rtti
 void rtti_set(rtti_t* r, int type, void* val);
 // cast type in rtti
 void rtti_cast(rtti_t* r, int type_id);
 // return pointer on data in rtti
 const void* rtti_ptr(const rtti_t* r);
 // free rtti_t
 void rtti_free(rtti_t** r);
```
- smartptr
``` c
// one_owner
    // create new pointer
    one_ownerptr* one_owner_create(size_t size);
    // move owning on pointer 
    one_ownerptr* one_owner_move(one_ownerptr* __dest__);
    // get raw pointer on data 
    void* one_owner_get(one_ownerptr* p);
    // realloc memory in pointer
    void one_owner_realloc(one_ownerptr* __ptr,size_t __newSize);
    // check pointer
    int one_owner_isvalid(one_ownerptr* p);
    // set value in pointer (macro)
    #define one_owner_set(p,t,v)
    // free pointer 
    void one_owner_free(one_ownerptr** p);
// shared
    // create new shared pointer 
    shared_ptr* shared_new(size_t size);
    // copy pointer
    shared_ptr* shared_copy(shared_ptr* targ);
    // get raw pointer 
    void* shared_get(shared_ptr* targ);
    // set value in pointer
    #define shared_set(p,t,v)
    // get references on pointer
    size_t shared_getrefs(shared_ptr* targ);
    // check pointer
    int shared_isvalid(shared_ptr* p);
    // free shared pointer
    void shared_free(shared_ptr** targ);
    // new weak pointer
    weak_ptr* weak_new(shared_ptr* targ);
    // change adress of weak pointer
    void weak_change(weak_ptr* targ,shared_ptr* new); 
    // lock weak pointer
    shared_ptr* weak_lock(weak_ptr* __ptr);
    // free weak pointer
    void weak_free(weak_ptr** __ptr);
```
- str
``` c
// create a new empty string
string_t* STRING_create(void);
// write 1 character to string Targ
void STRING_writeChar(string_t* Targ, char chr); 
//return pointer to C string in "trg"
/* WARNING : POINTER CAN BE INVALID AFTER CHANGE (e.g. STRING_writeChar) */  
const char* STRING_getString(string_t* trg);
// free dynamic string
void STRING_free(string_t** s);
//write C string to dynamic
void STRING_writeString(string_t* Targ, const char* Str);
// return pointer to found character or NULL
char* STRING_find(string_t* Targ, char targ);
// Change char in string to another .
/* flags
    __STRING_flag_all
    (change all found chars)
    __STRING_flag_default
    (change first found char)
*/
void STRING_rewriteChar(string_t* Targ, char old,char targ,unsigned char flag);
// clean string, but not free
void STRING_clear(string_t* Targ);
// get max chars 
size_t STRING_GetMax(string_t* Targ);
//get free chars (max - len)
size_t STRING_GetFree(string_t* Targ);
// get current len of string
size_t STRING_getln(string_t* Targ);

/* ===== MACROS ===== */
//max possible number (for computer)
#define STRING_AUTO ((size_t)-1)
//flags for function STRING_rewriteChar
#define __STRING_flag_all 1
#define __STRING_flag_default 0
```
- signal
``` c
    // create new signal
    signal_t* signal_new(void);
    // make chain with signals (WARN : don't make loops( else loops -> oops))
    void signal_chain(signal_t* target,signal_t* node);
    // connect handle to signal
    void signal_connect(signal_t* s,handle_t* fn);
    // emit signal with message ( message mustn't be NULL )
    void signal_emit(signal_t* s, const char* message, void** data, void* ret)
    // free signal ( freeing all handles)
    void signal_free(signal_t** s);
    // shared emit (to all signals)
    void signal_shared_emit(const char* message,void** d,void* r);
    // create new handle
    handle_t* handle_new(worker_sig fn);
    // set trigger message
    void handle_runOn(handle_t* h,const char* targs);
    // get count of runs of handle
    size_t handle_runs(handle_t* h);
    // set Max Runs of handle (if runs == max then handle not starts on signal)
    void handle_setMaxRuns(handle_t* h, size_t runs);
    // free handle ( and unregister from signal)
    void handle_free(handle_t** h);
    // get count of handles of signal
    size_t signal_handles(signal_t* s);
    // get count of signals
    size_t signal_total_signals();
    // create new group of signals
    signal_group_t* signal_new_group();
    // emit to all signals in group with message
    void signal_group_emit(signal_group_t* g,const char* msg,void** data,void* ret);
    // set trigger message of signal
    void signal_set_trigger_message(signal_t* s, const char* msg);
    // connect signal to group
    void signal_connect_to_group(signal_group_t * g,signal_t* s);
    // disconnect signal from group
    void signal_disconnect_from_group(signal_group_t* g,signal_t* s);
    // free signal group (WARN : signals not deleting)
    void signal_group_free(signal_group_t** g);
    // DEBUG FUNCTIONS
    // print info about handle
    void handle_dump(handle_t* h);
    // print info about signal
    void signal_dump(signal_t* s);

```
#### Examples
    search directory (folder) exams
##### Install
    ``` bash
        make && sudo make install && make clean
    ```
###### LICINSE
    MIT