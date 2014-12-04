#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
//#include <linux/moduleparam.h>
#include <linux/security.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/sched.h>

#define MAX_LENGTH 512 //单条规则最大长度
#define MAX_RULE_LENGTH 100 //最大规则数

#define MAX_AUTHORITY "2047\0" //最大权限值

//文件访问类
#define REMOVE_AUTHORITY 1 //删除文件权限值
#define MKDIR_AUTHORITY 2 //创建文件夹权限值
#define OPEN_AUTHORITY 4 //打开文件权限值
#define CREATE_AUTHORITY 8 //创建文件权限值
#define WRITE_AUTHORITY 16 //写文件权限值
#define EXEC_AUTHORITY 32 //执行文件权限值

//通信类
#define SOCKET_CREATE 256 //创建SOCKET权限值
#define SOCKET_CONNECT 512 //连接SOCKET权限值
#define SHARED_MEMORY 1024 //共享内存权限值

//管理类
#define MOUNT_AUTHORITY 64 //挂载文件系统权限值
#define UMOUNT_AUTHORITY 128 //卸载文件系统权限值

//规则集，字符串数组
char controlledRules[MAX_RULE_LENGTH][MAX_LENGTH];
int ruleNumber = 0;
int copyToUserRuleNumber = -1;

int enable_flag = 0;

static int get_fullpath(struct dentry *dentry, char *full_path)
{
	struct dentry *tmp_dentry = dentry;
	char tmp_path[MAX_LENGTH];
	char local_path[MAX_LENGTH];
	memset(tmp_path,0,MAX_LENGTH);
	memset(local_path,0,MAX_LENGTH);
	while (tmp_dentry != NULL)
	{
		if (!strcmp(tmp_dentry->d_iname,"/"))
			break;
		strcpy(tmp_path,"/");
		strcat(tmp_path,tmp_dentry->d_iname);
		strcat(tmp_path,local_path);
		strcpy(local_path,tmp_path);

		tmp_dentry = tmp_dentry->d_parent;
	}
	strcpy(full_path,local_path);
	return 0;
}

static char* get_current_process_full_path() {    
    if(current->mm) {
        struct vm_area_struct *vma = current->mm->mmap;
        while(vma) {
            if((vma->vm_flags && VM_EXECUTABLE) && vma->vm_file) {
                char *buffer = kmalloc(PAGE_SIZE, GFP_KERNEL);
                char *p;
                memset(buffer, 0, PAGE_SIZE);
                p = d_path(vma->vm_file->f_dentry, vma->vm_file->f_vfsmnt, buffer, PAGE_SIZE);
                if(!IS_ERR(p)) {
                    memmove(buffer, p, strlen(p) + 1);
                    //printk("%s\n", buffer);
                    return (char*) buffer;
                }
                kfree(buffer);
                return NULL;
            }
            vma = vma->vm_next;
        }
    }
    return NULL;
}

static char* get_process_full_path(struct task_struct * task) {    
    if(task->mm) {
        struct vm_area_struct *vma = task->mm->mmap;
        while(vma) {
            if((vma->vm_flags && VM_EXECUTABLE) && vma->vm_file) {
                char *buffer = kmalloc(PAGE_SIZE, GFP_KERNEL);
                char *p;
                memset(buffer, 0, PAGE_SIZE);
                p = d_path(vma->vm_file->f_dentry, vma->vm_file->f_vfsmnt, buffer, PAGE_SIZE);
                if(!IS_ERR(p)) {
                    memmove(buffer, p, strlen(p) + 1);
                    //printk("%s\n", buffer);
                    return (char*) buffer;
                }
                kfree(buffer);
                return NULL;
            }
            vma = vma->vm_next;
        }
    }
    return NULL;
}

int check(char* currentProcessFullPath, int authority) {
    if (enable_flag == 0) {
        return 0;
    }
    else if(currentProcessFullPath == NULL) {
        //printk("currentProcessFullPath is null\n");
        return 0;
    }
    int i;
    for (i = 0; i < ruleNumber; ++i) {
        if(strncmp(controlledRules[i], currentProcessFullPath, strlen(currentProcessFullPath)) == 0) {
            //切割字符串
            char r_authoriy[MAX_LENGTH];
            char* endptr;
            char r_path[MAX_LENGTH];
            strcpy(r_authoriy, controlledRules[i]);
            char* const delim = " "; 
            char *token, *cur = r_authoriy;
            int j = 0;
            while (token = strsep(&cur, delim)) {  
                if(j == 0) {
                    strcpy(r_path, token);
                }
                else if(j == 1) {
                    strcpy(r_authoriy, token);
                }
                j++;
            }

            
            unsigned long r_authoriy_unsigned_long = simple_strtol(r_authoriy, &endptr, 10);

            if((r_authoriy_unsigned_long & authority) == 0) {
                return 1;
            }
            else {
                return 0;
            }
        }
    }
    return 0;
}

static int lsm_inode_rmdir(struct inode *dir, struct dentry *dentry) {
	char* currentProcessFullPath = get_current_process_full_path();
    
    if(check(currentProcessFullPath, REMOVE_AUTHORITY) != 0) {
        printk("remove denied\n");
        return 1;
    }
    else {
        return 0;
    }
}

static int lsm_inode_unlink(struct inode *dir, struct dentry *dentry) {
    char* currentProcessFullPath = get_current_process_full_path();
    
    if(check(currentProcessFullPath, REMOVE_AUTHORITY) != 0) {
        printk("remove denied\n");
        return 1;
    }
    else {
        return 0;
    }
}

static int lsm_inode_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode) {
    char* currentProcessFullPath = get_current_process_full_path();
    
    if(check(currentProcessFullPath, MKDIR_AUTHORITY) != 0) {
        printk("mkdir denied\n");
        return 1;
    }
    else {
        return 0;
    }
}

static int lsm_inode_create(struct inode *dir, struct dentry *dentry, umode_t mode) {
    char* currentProcessFullPath = get_current_process_full_path();
    if(check(currentProcessFullPath, CREATE_AUTHORITY) != 0) {
        printk("create denied\n");
        return -1;
    }
    else {
        return 0;
    }
}


static int lsm_inode_permission(struct inode *inode, int mask) {
    char* currentProcessFullPath = get_current_process_full_path();
    if((mask == 4) && (check(currentProcessFullPath, OPEN_AUTHORITY) != 0)) {
        printk("open denied\n");
        return -1;
    }
    else {
        return 0;
    }
}


static int lsm_file_permission(struct file *file, int mask) {
    char* currentProcessFullPath = get_current_process_full_path();

    /*
    if(strncmp(currentProcessFullPath, "/home/fengkuangxiaxia/Desktop", strlen("/home/fengkuangxiaxia/Desktop")) == 0) {
        printk("%s %d\n", currentProcessFullPath, mask);
    }
    */
    
    /*
    if((mask == 4) && (check(currentProcessFullPath, OPEN_AUTHORITY) != 0)) {
        printk("open denied\n");
        return -1;
    }
    else if((mask == 4) && (check(currentProcessFullPath, CREATE_AUTHORITY) != 0)) {
        printk("create denied\n");
        return -1;
    }
    else */if((mask == 2) && (check(currentProcessFullPath, WRITE_AUTHORITY) != 0)) {
        printk("write denied\n");
        return -1;
    }
    else {
        return 0;
    }
}

static int lsm_task_create(unsigned long clone_flags) {
    char* currentProcessFullPath = get_current_process_full_path();

    if(check(currentProcessFullPath, EXEC_AUTHORITY) != 0) {
        printk("exec denied\n");
        return -1;
    }
    else {
        return 0;
    }
}

static int lsm_sb_mount(const char *dev_name, struct path *path, const char *type, unsigned long flags, void *data) {
    char* currentProcessFullPath = get_current_process_full_path();

    if(check(currentProcessFullPath, MOUNT_AUTHORITY) != 0) {
        printk("mount denied\n");
        return -1;
    }
    else {
        return 0;
    }
}

static int lsm_sb_umount(struct vfsmount * mnt, int flags) {
    char* currentProcessFullPath = get_current_process_full_path();

    if(check(currentProcessFullPath, UMOUNT_AUTHORITY) != 0) {
        printk("umount denied\n");
        return -1;
    }
    else {
        return 0;
    }
}

static int lsm_socket_create(int family, int type, int protocol, int kern) {
    char* currentProcessFullPath = get_current_process_full_path();

    if(check(currentProcessFullPath, SOCKET_CREATE) != 0) {
        printk("socket create denied\n");
        return -1;
    }
    else {
        return 0;
    }
}

static int lsm_socket_connect(struct socket *sock, struct sockaddr *address, int addrlen) {
    char* currentProcessFullPath = get_current_process_full_path();

    if(check(currentProcessFullPath, SOCKET_CONNECT) != 0) {
        printk("socket connect denied\n");
        return -1;
    }
    else {
        return 0;
    }
}

static int lsm_shm_shmat(struct shmid_kernel *shp, char __user *shmaddr, int shmflg) {
    char* currentProcessFullPath = get_current_process_full_path();
    
    if(check(currentProcessFullPath, SHARED_MEMORY) != 0) {
        printk("shared memory denied\n");
        return -1;
    }
    else {
        return 0;
    }
}


int write_controlledRules(int fd, char *buf, ssize_t len)
{
	char controlleddir[256]; 
    char controlledCommand[MAX_LENGTH];

	if (len == 0){
		enable_flag = 0;
		printk("Cancel the protect mechanism sucessfullly! \n");
		return len;
	}

	if (copy_from_user(controlledCommand, buf, len) != 0){
		printk("Can't get the controlled directory's name! \n");
		printk("Something may be wrong, please check it! \n");
		enable_flag = 0;
	}
	controlledCommand[len] = '\0';
    
	enable_flag = 1;

    //切割字符串
    
    char* const delim = " "; 
    char *token, *cur = controlledCommand;
    int i = 0;
    while (token = strsep(&cur, delim)) {  
        if(i == 0) {
            strcpy(controlleddir, token);
        }
        else if(i == 1) {
            strcpy(controlledCommand, token);
        }
        i++;
    }  
    
    //printk("Controlleddir name: %s \n", controlleddir);
    //printk("controlledCommand name: %s \n", controlledCommand);

    //未给权限值，表明给予所有权限
    if(strlen(controlledCommand) == 0) {
        memset(controlledCommand, 0, MAX_LENGTH);
        strcpy(controlledCommand, MAX_AUTHORITY);
        len += strlen(MAX_AUTHORITY);
    }
    
    int flag = 0;
    //更新现有规则
    for(i = 0; i < ruleNumber; i++) {
        if (strncmp(controlledRules[i], controlleddir, strlen(controlleddir)) == 0) {
            memset(controlledRules[i], 0, MAX_LENGTH);
            strcpy(controlledRules[i], controlleddir);
            strcat(controlledRules[i], " ");
            strcat(controlledRules[i], controlledCommand);
            controlledRules[i][len] = '\0';
            flag = 1;
            break;
        }
        
    }
    //添加新规则
    if(flag == 0) {
        strcpy(controlledRules[ruleNumber], controlleddir);
        strcat(controlledRules[ruleNumber], " ");
        strcat(controlledRules[ruleNumber], controlledCommand);
        controlledRules[ruleNumber][len] = '\0';
        ruleNumber++;
    }
	return len;
}


int read_controlledRules(int fd, char *buf, ssize_t len) {
    if(ruleNumber == 0) {
        copy_to_user(buf, "end", MAX_LENGTH);
        return;
    }
    copyToUserRuleNumber++;
    if(copyToUserRuleNumber == ruleNumber) {
        copyToUserRuleNumber = -1;
        copy_to_user(buf, "end", MAX_LENGTH);
    }
    else {
        copy_to_user(buf, controlledRules[copyToUserRuleNumber], strlen(controlledRules[copyToUserRuleNumber]));
    }
}

struct file_operations fops = {
	owner: THIS_MODULE, 
	write: write_controlledRules, 
    read: read_controlledRules,
}; 

static struct security_operations lsm_ops=
{
	.inode_create = lsm_inode_create,
    .inode_unlink = lsm_inode_unlink,
    .inode_mkdir = lsm_inode_mkdir,
    .inode_rmdir = lsm_inode_rmdir,
	.file_permission = lsm_file_permission,
    .inode_permission = lsm_inode_permission,
    .task_create = lsm_task_create,

    .socket_create = lsm_socket_create,
    .socket_connect = lsm_socket_connect,
    .shm_shmat = lsm_shm_shmat,

    .sb_mount = lsm_sb_mount,
    .sb_umount = lsm_sb_umount,
};

static int secondary=0;

static int __init lsm_init(void)
{
	int ret;
    if(register_security(&lsm_ops)) {
        printk(KERN_INFO"Failure registering LSM module with kernel\n");
        if(mod_reg_security(KBUILD_MODNAME, &lsm_ops)) {
            printk(KERN_INFO"Failure reigstering LSM module with primary module\n");
            return 1;
        }
        secondary=1;
    }

    printk(KERN_INFO"LSM Module Init Success! \n");
    ret = register_chrdev(123, "/dev/controlfile", &fops); 	// 向系统注册设备结点文件
	if (ret != 0) printk("Can't register device file! \n"); 
	
//	strcpy(controlleddir,"/root/lsm-check");
    return 0;
}

static void __exit lsm_exit(void)
{
    if(secondary) {
        mod_unreg_security(KBUILD_MODNAME,&lsm_ops);
    }
    else {
        unregister_security(&lsm_ops);
    }

    printk(KERN_INFO"LSM Module unregistered.....\n");
	unregister_chrdev(123, "procinfo");	 // 向系统注销设备结点文件 
	
}
//security_initcall(lsm_init);

module_init(lsm_init);
module_exit(lsm_exit);

MODULE_LICENSE("GPL");

