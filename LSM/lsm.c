#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
//#include <linux/moduleparam.h>
#include <linux/security.h>
#include <linux/fs.h>
//#include <linux/mm.h>
#include <linux/uaccess.h>

#define MAX_LENGTH 512
char controlleddir[256]; 
char controlledCommand[MAX_LENGTH]; 

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

int myown_check(char *full_name){
	if (enable_flag == 0)
		return 0;
	if (strncmp(full_name, controlleddir,strlen(controlleddir)) == 0)
	{
		printk("remove denied of the file: %s \n",full_name);
		return 1;
	} 
	else return 0;
}

static int lsm_inode_rmdir(struct inode *dir, struct dentry *dentry)
{
	char full_name[MAX_LENGTH];
//	printk(KERN_INFO"Function 'inode_rmdir' has been called\n");
	memset(full_name,0,MAX_LENGTH);
	get_fullpath(dentry,full_name);
	if (myown_check(full_name) != 0)
	{
		printk("remove denied of the directory: %s \n",full_name);
		return 1;
	} 
	return 0;
}

static int lsm_inode_unlink(struct inode *dir, struct dentry *dentry)
{
	char full_name[MAX_LENGTH];
//	printk(KERN_INFO"Function 'inode_unlink' has been called\n");
	memset(full_name,0,MAX_LENGTH);
	get_fullpath(dentry,full_name);
//	printk("fullname:%s  controlleddir:%s \n",full_name,controlleddir);
	if (myown_check(full_name) != 0)
	{
		printk("remove denied of the file: %s \n",full_name);
		return 1;
	} 
	return 0;
}

int write_controlleddir(int fd, char *buf, ssize_t len)
{
	
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
    
    printk("Controlleddir name: %s \n", controlleddir);
    printk("controlledCommand name: %s \n", controlledCommand);

	return len;
}




struct file_operations fops = {
	owner:THIS_MODULE, 
	write: write_controlleddir, 
}; 

static struct security_operations lsm_ops=
{
	.inode_rmdir = lsm_inode_rmdir,
	.inode_unlink = lsm_inode_unlink,
//	.inode_mkdir = lsm_inode_mkdir,
//	.inode_create = lsm_inode_create,
//	.file_alloc_security = lsm_file_alloc,
//	.file_free_security = lsm_file_free_security,
//	.file_permission = lsm_file_permission,
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

