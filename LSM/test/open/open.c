#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE* fp;
    if((fp = fopen("./test.txt", "r")) == NULL) {
        printf("Open fail.\n");
    }
    else {
        printf("Open sucess.\n");
        fclose(fp);
    }
    return 0;
}
