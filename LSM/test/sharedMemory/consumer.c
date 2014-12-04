#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>

int main(int argc, char* argv[])  
{  
    key_t key = ftok("./mark", 'a');  // 要保证两个进程的key_t，才能正确使用共享内存  
    int shmid = shmget(key, 100, 0600);  
    void* buf = (void*)shmat(shmid, NULL, 0);  
  
    printf("shared memory: %s\n", buf);  
    sleep(8);  
    printf("shared memory: %s\n", buf);  
  
    if ( -1 == shmdt(buf) ) {  
        fprintf(stderr, "%s\n", strerror(errno));  
        exit(1);  
    }  
  
    printf("consumer exited normally\n");  
    return 0;  
}  
