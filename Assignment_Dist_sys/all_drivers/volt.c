#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/random.h>
#include <linux/mman.h>
#include <linux/highmem.h>   // For kmap()
#include <linux/io.h>        // For ioremap()
#include <linux/gfp.h>       // For GFP_KERNEL

#define SHM_SIZE 4096
#define DEVICE_NAME "volt_dev"
#define TIMER_INTERVAL 3 * HZ

static int major;
static void *shm_region;
static struct timer_list random_timer;

struct data_entry {
    unsigned long timestamp;
    int value;
};

// Function to generate random data and store it in shared memory
static void generate_random_data(struct timer_list *t) {
    static int index = 0;
    struct data_entry *entry;

    entry = (struct data_entry *)((char *)shm_region + index *
sizeof(struct data_entry));
    entry->timestamp = jiffies;
    entry->value = 50 + (get_random_u32() % 71);
    pr_info("Current voltmeter is %d volt | Timestamp: %lu\n",entry->value, entry->timestamp);

    index = (index + 1) % (SHM_SIZE / sizeof(struct data_entry));

    mod_timer(&random_timer, jiffies + TIMER_INTERVAL);
}

// mmap handler for exposing shared memory to user space
static int device_mmap(struct file *file, struct vm_area_struct *vma) {
    unsigned long size = vma->vm_end - vma->vm_start;
    unsigned long pfn = virt_to_phys(shm_region) >> PAGE_SHIFT;

    if (size > SHM_SIZE)
        return -EINVAL;

    if (remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot))
        return -EAGAIN;

    return 0;
}

static int device_open(struct inode *inode, struct file *file) {
    pr_info("Device opened\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    pr_info("Device closed\n");
    return 0;
}

// Add this function before file_operations structure
static ssize_t device_write(struct file *file, const char __user *buf,
                           size_t count, loff_t *pos) {
    char alert[256];
    int ret;

    if (count > sizeof(alert) - 1)
        return -EINVAL;

    ret = copy_from_user(alert, buf, count);
    if (ret)
        return -EFAULT;

    alert[count] = '\0';
    // Print the alert to kernel log
    pr_alert("ALERT from userspace: %s", alert);

    return count;
}

// Modify the file_operations structure to include write
static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .mmap = device_mmap,
    .write = device_write,  // Add this line
};


// Device initialization function
static int __init device_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register device\n");
        return major;
    }

    pr_info("Major number assigned: %d\n", major);

    // Allocate shared memory with page alignment
    shm_region = (void *)__get_free_pages(GFP_KERNEL, get_order(SHM_SIZE));
    if (!shm_region) {
        unregister_chrdev(major, DEVICE_NAME);
        return -ENOMEM;
    }

    pr_info("Shared memory allocated at %p\n", shm_region);

    // Initialize and start the timer
    timer_setup(&random_timer, generate_random_data, 0);
    mod_timer(&random_timer, jiffies + TIMER_INTERVAL);

    pr_info("Device loaded\n");
    return 0;
}

// Device exit function
static void __exit device_exit(void) {
    del_timer_sync(&random_timer);
    free_pages((unsigned long)shm_region, get_order(SHM_SIZE));
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Device unloaded\n");
}

module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL");




