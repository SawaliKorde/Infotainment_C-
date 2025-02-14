//This is kernel driver code

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define DEVICE_NAME "shm_writer_device"
#define BUFFER_SIZE 1024
#define MEM_BASE_ADDR 0x0 //Address
#define MEM_SIZE 4096 
#define MAJOR_NUM 275  // Specify your desired major number here

static void __iomem *mem_base;
static char buffer[BUFFER_SIZE];
static int buffer_size = 0;

static int device_open(struct inode *inode, struct file *file)
{
    // Safely map physical memory region
    mem_base = ioremap(MEM_BASE_ADDR, MEM_SIZE);
    if (!mem_base) {
        printk(KERN_ERR "Failed to map memory region\n");
        return -ENOMEM;
    }
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    if (mem_base) {
        iounmap(mem_base);
    }
    return 0;
}

static ssize_t device_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset)
{
    if (!mem_base) return -EINVAL;
    
    // Read from mapped memory
    memcpy_fromio(buffer, mem_base, len);
    
    if (copy_to_user(user_buffer, buffer, len))
        return -EFAULT;
        
     printk(KERN_INFO "DEVICE READ %s \n", buffer);
    return len;
}

static ssize_t device_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *offset)
{
    if (!mem_base) return -EINVAL;
    if(len > BUFFER_SIZE)
       len = BUFFER_SIZE;
        
    if (copy_from_user(buffer, user_buffer, len))
        return -EFAULT;
        
    // Write to mapped memory
    buffer[len] = '\0';
    memcpy_toio(mem_base, buffer, len);
    
    printk(KERN_INFO "Data written to device %s\n", buffer);
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

static int __init shm_writer_device_init(void)
{
    int result = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
    if (result < 0) {
        printk(KERN_ALERT "Failed to register device\n", result);
        return result;
    }
    printk(KERN_INFO "Device registered with major number %d\n",MAJOR_NUM );
    return 0;
}

static void __exit shm_writer_device_exit(void)
{
    unregister_chrdev(MAJOR_NUM , DEVICE_NAME);
    if (mem_base) {
        iounmap(mem_base);
    }
}

module_init(shm_writer_device_init);
module_exit(shm_writer_device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sawali Korde");
MODULE_DESCRIPTION("Safe memory access device driver");

