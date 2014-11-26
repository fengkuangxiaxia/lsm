#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char filename[256]; //要控制的程序路径
	char operation[256];    //对程序的控制行为
    char result[512];   //最后的完整控制字符串
	int fd; //设备文件打开时用到的文件描述符 
	struct stat buf;    //文件状态结构缓冲区 
	
	if (argc == 1) {    //要控制的程序路径为空，去除所有保护
		*filename = '\0';
		*operation = '\0';
	}
	else {
		if (argc == 2){ //控制行为为空，关闭对应程序的保护功能
			if (strlen(argv[1]) >= 256) {   //容错性检查
				printf("The controlled path is too long! please check it and try again! \n");
				exit(1);
			}
			strcpy(filename, argv[1]);  //获取要控制的程序路径
			if (stat(filename,&buf) != 0) { //检查要控制的程序是否存在
				printf("The file(or directory) may not exist! \n");
				exit(1);
			}
			*operation = '\0';
		}
		else if(argc == 3) {
			if (strlen(argv[1]) >= 256) {   //容错性检查
				printf("The controlled path is too long! please check it and try again! \n");
				exit(1);
			}
			strcpy(filename, argv[1]);  //获取要控制的程序路径
			if (stat(filename,&buf) != 0) { //检查要控制的程序是否存在
				printf("The file(or directory) may not exist! \n");
				exit(1);
			}
			if (strlen(argv[2]) >= 256) {   //容错性检查
				printf("The controlled command is too long! please check it and try again! \n");
				exit(1);
			}
			strcpy(operation, argv[2]); //获取控制命令
		}
		else {  //参数输入错误，提示正确的命令行参数
			printf("Commandline parameters are wrong! please check them! \n");
			printf("Usage: %s, or %s file(directory)name, or %s file(directory)name controllCommand! \n", argv[0], argv[0], argv[0]);
			exit(1);
		}
	}
    
    if((strlen(filename) + strlen(operation) + 1) >= 512) {
        printf("The full command is too long! please check it and try again! \n");
        exit(1);
    }
    else {
        *result = '\0';
        strcat(result, filename);
        strcat(result, " ");
        strcat(result, operation);
    }

	//printf("%s \n", result);

	
	if (stat("/dev/controlfile",&buf) != 0) {
		//探测设备文件是否已经创建，如果没有创建，则先创建该设备文件
		if (system("mknod /dev/controlfile c 123 0") == -1){
			printf("Cann't create the devive file ! \n");
			printf("Please check and try again! \n");
			exit(1);
		}
	}
	fd =open("/dev/controlfile",O_RDWR,S_IRUSR|S_IWUSR);    //打开设备文件
	if (fd > 0) {
		write(fd,result,strlen(result));    //写入数据
	}
	else {
		perror("can't open /dev/controlfile \n");
	 	exit (1);
	}
	close(fd);  //关闭设备文件
}

