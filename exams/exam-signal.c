#include <Aurora/signal.h>
#include <stdio.h>
void on_click(void** args,void* r) { // create handler
    *(int*)r = (*(int*)args[0]) + (*(int*)args[1]);
    puts("click!");
}
void print_msgs(void** args,void* r) { // handler
    puts(args[0]);
}
int main() {
    signal_t* s = signal_new(); // create signal
    int f = 89;
    int s2 = 67;
    int ret = 0;
    void* d[] = {&f,&s2};
    char * st = "Hello World";
    handle_t* h = handle_new(on_click); // new handlers
    handle_t* h2 = handle_new(print_msgs);

    handle_runOn(h,"click");
    handle_runOn(h2,"print"); // set filters

    handle_setMaxRuns(h,5); // set max runs

    signal_connect(s,h);
    signal_connect(s,h2); // connect handlers

    for (int i = 0; i <= 25; i++) {
        d[0] = &f;
        signal_emit(s,"click",d,&ret); // emit messages
        d[0] = st;
        signal_emit(s,"print",d,NULL);
        printf("%d\n",ret);
    }
    signal_dump(s); // dump of signal
    handle_free(&h);
    handle_free(&h2); // free all
    signal_free(&s);
}