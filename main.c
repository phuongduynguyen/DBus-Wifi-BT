/**
 * Before run:
 * g++ -o main main.c -I./Lib/include/public -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -L. -lNetwork -ldbus-1
 * chmod +r libNetwork.so
 * export LD_LIBRARY_PATH=.
 * Start run:
 * ./main
 */


#include "Lib/include/public/NetworkProvider.h"
#include <dlfcn.h>
#include <stdio.h>

int main(void)
{
    void* handler = dlopen("./libNetwork.so", RTLD_LAZY);
    if (!handler) {
        printf("Cant open libNetwork.so : %s\n", dlerror());
        return 1;
    }

    typedef void* (*init_func)();
    init_func np_initalize = (init_func)dlsym(handler, "np_initialize");

    typedef void (*toggle_func_t)(void*, int); 
    toggle_func_t np_toggle_network = (toggle_func_t)dlsym(handler, "np_toggle_network");
    
    char* error = dlerror();
    if (error) {
        printf("Cant open libNetwork.so : %s\n", error);
        dlclose(handler);
        return 1;
    }

    void* np = np_initialize();
    np_toggle_network(np,0);
}