#include <stdio.h>

int main(int argc, char *argv[]) {
    if(remove("test.txt")) {
        printf("Remove fail.\n");
    }
    else {
        printf("Remove sucess.\n");
    }
    return 0;
}
