#include <stdio.h>
#include "cutils-shared_ptr.h"
int main() {
    // new shared pointer 
    shared_ptr * shar = shared_new(sizeof(double)); 
    printf("refs on shar : %lu\n",shared_getrefs(shar)); // refs on shar : 1
    weak_ptr * t = weak_new(shar); // new weak pointer
    printf("refs on shar : %lu\n",shared_getrefs(shar)); // refs on shar : 1
    shared_ptr* test = weak_lock(t); // cheate new shared pointer
    printf("refs on shar : %lu\n",shared_getrefs(shar)); // refs on shar : 2
    shared_free(&shar); // free shar
    shared_free(&test); // free test
    weak_free(&t); // free weak pointer
    return 0;
}