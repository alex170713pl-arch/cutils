#include "../include/cutils-str.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
struct string{
    char* Str;
    size_t len;
    size_t max;
};
string_t* STRING_create() {
    string_t* new = calloc(1,sizeof(struct string));
    if (!new) return NULL;
    new->Str = calloc(1,10);
    if (!new->Str) {
        free(new);
        return NULL;
    }
    new->max = 10;
    new->len = 0;
    new->Str[new->len] = '\0';
    return new;
}
void STRING_writeChar(string_t* Targ,char chr) {
    if (!Targ || !Targ->Str || Targ->max >= (SIZE_MAX /2)) {return;}
    if (Targ->len + 1 == Targ->max) {
        size_t NewMax = Targ->max * 2;
        char* ptr = realloc(Targ->Str,NewMax);
        if(!ptr) {return;}
        Targ->Str = ptr;
        Targ->max = NewMax;
    }
    Targ->Str[Targ->len] = chr;
    Targ->len++;
    Targ->Str[Targ->len] = '\0';
}

const char* STRING_getString(string_t* trg) {
    if(!trg || !trg->Str) {return NULL;}
    return trg->Str;
}
size_t STRING_GetFree(string_t* Targ) {
    return (Targ->max - Targ->len) - 1;
}
size_t STRING_GetMax(string_t* Targ) {
    return Targ->max;
}
size_t STRING_getln(string_t* Targ) {
    return Targ->len;
}
void STRING_writeString(string_t* Targ, const char* Str) {
    if (!Targ || !Targ->Str || !Str) return;
    
    size_t ln = strlen(Str);
    if (Targ->len + ln + 1 > Targ->max) {
        size_t new_max = Targ->len + ln + 1;
        char* new_str = realloc(Targ->Str, new_max);
        if (!new_str) return;
        Targ->Str = new_str;
        Targ->max = new_max;
    }
    
    memmove(Targ->Str + Targ->len, Str, ln + 1);
    Targ->len += ln;
}
char* STRING_find(string_t* Targ, char targ) {
    if (!Targ || !Targ->Str) return NULL;
    return strchr(Targ->Str,targ);
}
void STRING_rewriteChar(string_t* Targ, char old,char targ,unsigned char flag) {
    if (!Targ || !Targ->Str) return;
    switch (flag) {
        case 0 : {
            char* trg = STRING_find(Targ,old);
            if (!trg) return;
            *trg = targ;
        }
        break;
        case 1: {
            size_t i = 0;
            for (i = 0; i < Targ->len;i++) {
                if (Targ->Str[i] == old)
                    Targ->Str[i] = targ;
            }
        }
        break;
    }
}
void STRING_clear(string_t* Targ) {
    if (!Targ) return;
    free(Targ->Str);
    Targ->Str = calloc(10,sizeof(char));
    if (!Targ->Str) {
        Targ->len = 0;
        Targ->max = 0;
        return;
    }
    Targ->len = 0;
    Targ->max = 10;
}
void STRING_free(string_t** s) {
    if(!(*s) || !(*s)->Str) {return;}
    free((*s)->Str);
    (*s)->Str = NULL;
    (*s)->len = 0;
    (*s)->max = 0;
    free(*s);
    *s = NULL;
}
