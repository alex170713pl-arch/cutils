#ifndef AURORA_OBJ

    #define AURORA_OBJ
    typedef struct obj_class obj_class_t;
    typedef struct obj obj_t;
    typedef struct obj_interf obj_interface_t;
    #include <stddef.h>
    #include "type.h"
    typedef enum {
        ACCESS_PUBLIC,
        ACCESS_PRIVATE,
        ACCESS_PROTECTED
    } access_level;
    typedef void(*obj_met)(obj_t * self,void* sec,void** args);
    typedef void(*obj_constructor)(obj_t * self,void* sec,void** args);
    typedef void(*obj_destructor)(obj_t* self,void* sec);
    /*====== CLASS API =====*/

    obj_class_t* obj_new_class(const char * name,
        obj_constructor __init__,
        obj_destructor __del__
    );
    void obj_class_new_field(obj_class_t * class,int type,const char* name,access_level level);
    void obj_class_add_method(obj_class_t * class,obj_met method,const char* method_name,access_level level);
    int obj_class_extend(obj_class_t * who,obj_class_t * from);
    int obj_extend_interface(obj_class_t * who,obj_interface_t * from);
    void obj_class_virtual(obj_class_t * who,const char* method_name);
    void obj_class_abstract(obj_class_t* targ);
    int obj_class_override(obj_class_t * class , const char* met_name,obj_met new_method);
    void obj_class_free(obj_class_t** class); 
    /*===== OBJECTECTS API =====*/

    obj_t * obj_new(obj_class_t * class,void** constuct_args);
    void* obj_call(obj_t * o,const char* metName,void** args);
    int obj_get_field_val(obj_t * o,const char* field_name,void* bf,size_t size_of_field);
    int obj_set_field_val(obj_t* o,const char* field_name,void* from,size_t size_of_field);
    void obj_free(obj_t** obj);
    obj_t* obj_copy(obj_t* o);
    obj_t* obj_move(obj_t** o);
    obj_class_t* obj_get_class(obj_t* o);
    int obj_find_field(obj_t* o,const char* field_name);
    int obj_find_method(obj_t* o,const char* method_name);
    int obj_is(obj_t* o,obj_class_t* class_to_check);
    const char* obj_class_name(obj_t * o);
    void * obj_get_private(obj_t* o,void* sec,const char* field_name);
    void obj_set_private(obj_t* o,void* sec,void* raw_field,void* ptr,size_t datas);
    #define OBJ_SET_PRIVATE(o,sec,type,val,field_name) do {\
        type __template = val; \
        type* field = obj_get_private(o,sec,field_name) ;\
        obj_set_private(o,sec,field,&__template,sizeof(__template)); \
    } while(0)
    /*===== INTERFACES API =====*/

    obj_interface_t * obj_interface_new(const char* interf_name);
    int obj_interface_add_method(obj_interface_t * inter,const char* method_name);
    void obj_interface_free(obj_interface_t** inter);
#endif