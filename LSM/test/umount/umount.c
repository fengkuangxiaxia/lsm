#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/mount.h>

int main(int argc, char *argv[]) {
    const char *src = "/dev/cdrom";
    const char *trgt = "/mnt/cdrom";
    const char *type = "iso9660";
    const unsigned long mntflags = MS_RDONLY;
    const char *opts = "mode=0700,uid=65534";

    int result = umount(trgt);

    if(result == 0) {
        printf("Umount sucess\n");
    }
    else {
        printf("Umount fail\n");
        printf("%s [%d]\n", strerror(errno), errno);
        return -1;
    }
    return 0;
}
