#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define DEVICE_NAME "char_device"
#define BUFFER_SIZE 1024

static char device_buffer[BUFFER_SIZE];
static int buffer_size = 0;
static int major_number;
static struct cdev my_cdev;
static struct class *cls;

// Open function
static int device_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device opened\n");
    return 0;
}

// Release function
static int device_release(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device closed\n");
    return 0;
}

// Write function
static ssize_t device_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset) {
    if (count > BUFFER_SIZE) count = BUFFER_SIZE;

    if (copy_from_user(device_buffer, user_buffer, count)) {
        return -EFAULT;
    }

    buffer_size = count;
    printk(KERN_INFO "Received %zu bytes from user: %s\n", count, device_buffer);
    return count;
}

// Read function
static ssize_t device_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset) {
    if (*offset >= buffer_size) return 0; // EOF

    if (count > buffer_size - *offset) {
        count = buffer_size - *offset;
    }

    if (copy_to_user(user_buffer, device_buffer + *offset, count)) {
        return -EFAULT;
    }

    *offset += count;
    printk(KERN_INFO "Sent %zu bytes to user\n", count);
    return count;
}

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .write = device_write,
    .read = device_read,
};

// Module initialization
static int __init device_init(void) {
    dev_t dev;
    
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        return -1;
    }
    
    major_number = MAJOR(dev);
    printk(KERN_INFO "Device registered with major number %d\n", major_number);

    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    
    if (cdev_add(&my_cdev, dev, 1) < 0) {
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    cls = class_create("char_class");
    if (IS_ERR(cls)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(cls);
    }

    device_create(cls, NULL, dev, NULL, DEVICE_NAME);
    printk(KERN_INFO "Device created: /dev/%s\n", DEVICE_NAME);
    
    return 0;
}

// Module exit
static void __exit device_exit(void) {
    device_destroy(cls, MKDEV(major_number, 0));
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major_number, 0), 1);
    printk(KERN_INFO "Device unregistered\n");
}

module_init(device_init);
module_exit(device_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple character device driver");





