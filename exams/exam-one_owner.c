#include <Aurora/one_owner.h>
#include <stdio.h>

int main() 
{
    one_ownerptr * test = one_owner_create(sizeof(int)); // create new pointer with size of int
    one_owner_set(test,int,897); // set data in pointer
    printf("data of test: %d\n",*(int*)one_owner_get(test));
    
    for (int i = 0;i < 10 ;i++)
        one_owner_free(&test); // free pointer 10 times
    return 0;
}