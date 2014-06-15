#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/net.h>
#include <linux/wait.h>
#include <linux/namei.h>
#include <linux/file.h>
#include <linux/mman.h>
#include <linux/swap.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/pagemap.h>
#include <linux/vmalloc.h>
#include <linux/utsname.h>
#include <linux/smp_lock.h>
#include <linux/kernel_stat.h>
#include <linux/kernel_stat.h>
#include <linux/time.h>
#include <asm/div64.h>
#include <asm/unaligned.h>
#include <linux/compiler.h>
#include <linux/mount.h>
#include <linux/zlib.h>
#include <linux/syscalls.h>
#include <linux/cpumask.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daymion Reynolds <daymion@gmail.com>");
MODULE_DESCRIPTION("In-kernel Cache");
//module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
//MODULE_PARM_DESC(buffer_size, "Internal buffer size");

#define CONFIG_TUX_NUMTHREADS 16
#define NR_IO_THREADS 64
#define CONFIG_TUX_NUMSOCKETS 16
#define MOONRAKER_LOGGING 5

typedef struct moonraker_error{
	int level;
	char* msg;
}moonraker_error;

typedef struct moonraker_req{
	char *url;
	int size;
	int duration;
	int time;
	moonraker_error *error;
}moonraker_req;


void moon_log(moonraker_req *req);
moonraker_req *moon_req_alloc(void);
void moon_req_free(moonraker_req *req);
void moon_req_free_error(moonraker_req *req);
void moon_req_add_error(moonraker_req *req,int level,char* msg);
void moon_req_add_url(moonraker_req *req,char *url);

static int __init moonraker_init(void)
{
	printk(KERN_INFO "Moonraker device has been registered\n");
	return 0;
}
 
static void __exit moonraker_exit(void)
{
	printk(KERN_INFO "Moonraker device has been unregistered\n");
}
//Moonraker loger
void moon_log(moonraker_req *req)
{
	if(req->error->level<MOONRAKER_LOGGING)
               return;

	//Log it
}

//Moonraker Request memory managment
moonraker_req *moon_req_alloc(void)
{
	moonraker_req *req = kmalloc(sizeof(moonraker_req),GFP_KERNEL);
	req->url=NULL;
	req->error=NULL;
	return req;	

}

void moon_req_add_url(moonraker_req *req,char *url)
{
	req->url = kmalloc(strlen(url)+1,GFP_KERNEL);
	strcpy(req->url,url);
}

void moon_req_free(moonraker_req *req)
{
	if(req->url!=NULL)
		kfree(req->url);

	moon_req_free_error(req);

	kfree(req);

}
void moon_req_free_error(moonraker_req *req)
{
        if(req->error!=NULL)
        {
                if(req->error->msg!=NULL)
                        kfree(req->error->msg);
                kfree(req->error);		
        }
	req->error=NULL;
}

void moon_req_add_error(moonraker_req *req, int level, char *msg)
{
	moonraker_error *error;
	moon_req_free_error(req);
	error = kmalloc(sizeof(moonraker_error),GFP_KERNEL);
	error->level = level;
	error->msg = kmalloc(strlen(msg)+1,GFP_KERNEL);
	strcpy(error->msg,msg);
	req->error = error;
}
//end Moonraker Request memory managment

module_init(moonraker_init);
module_exit(moonraker_exit);
