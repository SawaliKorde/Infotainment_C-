#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/timekeeping.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mman.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>

#define SHM_SIZE 4096
#define DEVICE_NAME "random_dev"
#define MAJOR_NUM 282

static struct task_struct *task;
static void *shm_region;
static int major;
static dma_addr_t dma_handle;

struct data_entry {
    unsigned long timestamp;
    int value;
};

static int device_mmap(struct file *file, struct vm_area_struct *vma)
{
    unsigned long size = vma->vm_end - vma->vm_start;

    if (size > SHM_SIZE)
        return -EINVAL;

    /* Remap kernel memory to user space */
    if (remap_pfn_range(vma,
                        vma->vm_start,
                        virt_to_phys(shm_region) >> PAGE_SHIFT,
                        size,
                        vma->vm_page_prot)) {
        return -EAGAIN;
    }

    return 0;
}

static int device_open(struct inode *inode, struct file *file) {
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    return 0;
}

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .mmap = device_mmap,
};

static int data_generator_thread(void *arg) {
    struct data_entry *data = (struct data_entry *)shm_region;
    int index = 0;

    while (!kthread_should_stop()) {
        msleep(3000);

        unsigned long ts = ktime_get_real_seconds();
        int random_value;
        get_random_bytes(&random_value, sizeof(random_value));
        random_value = random_value % 1000;

        data[index].timestamp = ts;
        data[index].value = random_value;
        index = (index + 1) % (SHM_SIZE / sizeof(struct data_entry));

        pr_info("Device generated: %d at %lu\n", random_value, ts);
    }
    return 0;
}

static int __init device_init(void) {
    major = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);

    // Use coherent DMA memory allocation
    shm_region = dma_alloc_coherent(NULL, SHM_SIZE, &dma_handle, GFP_KERNEL);
    if (!shm_region) return -ENOMEM;

    memset(shm_region, 0, SHM_SIZE);

    task = kthread_run(data_generator_thread, NULL, "data_generator");
    if (IS_ERR(task)) {
        dma_free_coherent(NULL, SHM_SIZE, shm_region, dma_handle);
        return PTR_ERR(task);
    }

    pr_info("Device loaded\n");
    return 0;
}

static void __exit device_exit(void) {
    kthread_stop(task);
    unregister_chrdev(major, DEVICE_NAME);
    dma_free_coherent(NULL, SHM_SIZE, shm_region, dma_handle);
    pr_info("Device unloaded\n");
}

module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL");
