#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>

static void mysleep(int seconds) {  
    for ( ; seconds > 0; seconds-- ) {  
        sleep(1); printf("sleeping(%d)\n", seconds);  
    }  
}  
  
int main() {  
    key_t key = ftok("./mark", (int)'a');  
    int shmid = shmget(key, 100, IPC_CREAT | 0600);  
    void* buf = (void*)shmat(shmid, NULL, 0);  
        if ( (int)buf == -1 ) {  
        shmctl(shmid, IPC_RMID, NULL);  
        printf("producer fail\n");
        exit(1);  
    }  
  
    sprintf(buf, "hello, world\n");  
    mysleep(5);  
    sprintf(buf,"hello, world 2\n");  
    mysleep(5);  
  
    if ( -1 == shmdt(buf) ) {  
        fprintf(stderr, "%s\n", strerror(errno));  
        shmctl(shmid, IPC_RMID, NULL);  
        exit(1);  
    }  
  
    if ( -1 == shmctl(shmid, IPC_RMID, NULL) ) {  
        fprintf(stderr, "%s\n", strerror(errno));  
        exit(1);  
    }  
  
    printf("producer exited normally\n");  
    return 0;  
}  
