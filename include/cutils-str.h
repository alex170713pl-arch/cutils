#ifndef STR_H
#define STR_H

#include <stddef.h>   

#ifndef STRING_AUTO
#define STRING_AUTO ((size_t)-1)
#endif
#ifndef __STRING_flag_all 
    #define __STRING_flag_all 1
#endif
#ifndef  __STRING_flag_default
    #define __STRING_flag_default 0
#endif 
typedef struct string string_t;

string_t* STRING_create(void);
void STRING_writeChar(string_t* Targ, char chr);
const char* STRING_getString(string_t* trg);
void STRING_free(string_t** s);
void STRING_writeString(string_t* Targ, const char* Str);
char* STRING_find(string_t* Targ, char targ);
void STRING_rewriteChar(string_t* Targ, char old,char targ,unsigned char flag);
void STRING_clear(string_t* Targ);
size_t STRING_GetMax(string_t* Targ);
size_t STRING_GetFree(string_t* Targ);
size_t STRING_getln(string_t* Targ);

#endif