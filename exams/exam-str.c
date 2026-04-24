#include "cutils-str.h"
#include <stdio.h>
char hi[15] = "Hello World";
int main() {
    string_t* s = STRING_create(); // create new string
    if (!s) return 1; // if s = NULL
    STRING_writeString(s,hi); // write string Hello World 
    STRING_rewriteChar(s,'l','x',__STRING_flag_all); // change char l to char x
    puts(STRING_getString(s)); // Hexxo Worxd
    STRING_free(&s); // free string
    return 0;
}