#include <stdio.h>

int main(int argc, char *argv[]) {
    if(system("ls") == -1) {
        printf("Exec fail.\n");
    }
    else {
        printf("Exec sucess\n");
    }
    return 0;
}
