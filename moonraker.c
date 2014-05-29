#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daymion Reynolds <daymion@gmail.com>");
MODULE_DESCRIPTION("In-kernel Cache");
static unsigned long buffer_size = 8192;
module_param(buffer_size, ulong, (S_IRUSR | S_IRGRP | S_IROTH));
MODULE_PARM_DESC(buffer_size, "Internal buffer size");


static int __init reverse_init(void)
{
    if (!buffer_size)
        return -1;
    printk(KERN_INFO "Moonraker device has been registered, buffer size is %lu bytes\n",
        buffer_size);
    return 0;
}
 
static void __exit reverse_exit(void)
{
    printk(KERN_INFO "Moonraker device has been unregistered\n");
}
 
module_init(reverse_init);
module_exit(reverse_exit);
