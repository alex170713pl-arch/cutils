#include "cutils-rtti.h"
#include <stdio.h>
typedef struct {
    int x;
    int y;
} point;
int main() {
    rtti_begin(); // start rtti
        rtti_register("point",sizeof(point)); // register new type
        rtti_t* test = rtti_new_custom("point"); // create by name
        point z; // create new struct
        int type = rtti_typeid(NULL,test); // type = 24(25) 
        rtti_set(test,type,&z);
        printf("typeid : %d , typename : %s\n",
            type,
            rtti_typeof(test)
        ); // printf typeid and typename
        rtti_free(&test); // free rtti object
    rtti_end(); // end rtti
    return 0;
}