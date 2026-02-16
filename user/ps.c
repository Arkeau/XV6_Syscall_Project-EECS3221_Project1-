#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"


int
main(void)
{
    struct procinfo procs[64];

    int n = getprocs(procs, 64); 

    if(n < 0){
        printf("getprocs failed\n");
        exit(1);
    }

    for(int i = 0; i < n; i++){
        printf("%d %d %d %d %s\n",
               procs[i].pid,
               procs[i].ppid,
               procs[i].state,
               procs[i].sz,
               procs[i].name);
    }

    exit(0);
}
