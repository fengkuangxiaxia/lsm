#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE* fp;
    if((fp = fopen("./test.txt", "w")) == NULL) {
        printf("Create fail.\n");
    }
    else {
        printf("Create sucess.\n");
    }
    fclose(fp);
    return 0;
}
