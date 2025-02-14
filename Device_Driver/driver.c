#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "simple_device"
#define BUFFER_SIZE 1024
#define MAJOR_NUM 273  // Specify your desired major number here

static char buffer[BUFFER_SIZE];
static int buffer_size = 0;

static int device_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "simple_char_device: Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
  printk(KERN_INFO "simple_char_device: Device closed\n");
    return 0;
}

static ssize_t device_read(struct file *file, char __user *user_buffer, size_t len, loff_t *offset)
{
    int bytes_read = len < buffer_size ? len : buffer_size;
    if (copy_to_user(user_buffer, buffer, bytes_read))
        return -EFAULT;
    buffer_size -= bytes_read;
    memmove(buffer, buffer + bytes_read, buffer_size);
    return bytes_read;
}

static ssize_t device_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *offset)
{
    int bytes_to_write = len < (BUFFER_SIZE - buffer_size) ? len : (BUFFER_SIZE - buffer_size);
    if (copy_from_user(buffer + buffer_size, user_buffer, bytes_to_write))
        return -EFAULT;
    buffer_size += bytes_to_write;
    return bytes_to_write;
}

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

static int __init simple_char_driver_init(void)
{
    int result = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
    if (result < 0) {
        printk(KERN_ALERT "simple_char_device: Failed to register a major number\n");
        return result;
    }
    printk(KERN_INFO "simple_char_device: Registered device with major number %d\n", MAJOR_NUM);
    return 0;
}

static void __exit simple_char_driver_exit(void)
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_INFO "simple_char_device: Unregistered device\n");
}

module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sawali Korde");
MODULE_DESCRIPTION("A simple character device driver");
