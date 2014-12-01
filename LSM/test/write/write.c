#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE* fp;
    if((fp = fopen("./test.txt", "r")) == NULL) {
        printf("Read fail.\n");
    }
    else {
        printf("Read sucess.\n");
    }
    if((fp = fopen("./test.txt", "w")) == NULL) {
        printf("Open fail.\n");
    }
    else {
        printf("Open sucess.\n");
        fprintf(fp, "aaa\n");
        printf("Write sucess.\n");
        fclose(fp);
    }
    return 0;
}
