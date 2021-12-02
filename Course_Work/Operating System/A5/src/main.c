#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include "ioc_hw5.h"

MODULE_LICENSE("GPL");

#define PREFIX_TITLE "OS_AS5"

// CDEV:
static int dev_major;
static int dev_minor;
static struct cdev *dev_cdevp;

// DMA
//Those ports can be seen as offsets of the pointer.
#define DMA_BUFSIZE 64
#define DMASTUIDADDR 0x0        // Student ID
#define DMARWOKADDR 0x4         // RW function complete
#define DMAIOCOKADDR 0x8        // ioctl function complete
#define DMAIRQOKADDR 0xc        // ISR function complete
#define DMACOUNTADDR 0x10       // interrupt count function complete
#define DMAANSADDR 0x14         // Computation answer
#define DMAREADABLEADDR 0x18    // READABLE variable for synchronize
#define DMABLOCKADDR 0x1c       // Blocking or non-blocking IO
#define DMAOPCODEADDR 0x20      // data.a opcode
#define DMAOPERANDBADDR 0x21    // data.b operand1
#define DMAOPERANDCADDR 0x25    // data.c operand2
void *dma_buf;

//Work Queue
static struct work_struct *work;

// Declaration for file operations
static ssize_t drv_read(struct file *filp, char __user *buffer, size_t, loff_t*);
static int drv_open(struct inode*, struct file*);
static ssize_t drv_write(struct file *filp, const char __user *buffer, size_t, loff_t*);
static int drv_release(struct inode*, struct file*);
static long drv_ioctl(struct file *, unsigned int , unsigned long );

//Declaration for math operations
static int prime(int, short);

// cdev file_operations, this struct defined in Linux Kernel.
static struct file_operations drv_fops = {
      owner: THIS_MODULE,
      read: drv_read,
      write: drv_write,
      unlocked_ioctl: drv_ioctl,
      open: drv_open,
      release: drv_release,
};

// in and out function
void myoutc(unsigned char data,unsigned short int port);
void myouts(unsigned short data,unsigned short int port);
void myouti(unsigned int data,unsigned short int port);
unsigned char myinc(unsigned short int port);
unsigned short myins(unsigned short int port);
unsigned int myini(unsigned short int port);


// For input data structure
struct DataIn {
    char a;
    int b;
    short c;
} *dataIn;


// Arithmetic funciton
static void drv_arithmetic_routine(struct work_struct* ws);


// Input and output data from/to DMA
void myoutc(unsigned char data,unsigned short int port) {
    *(volatile unsigned char*)(dma_buf+port) = data;
}
void myouts(unsigned short data,unsigned short int port) {
    *(volatile unsigned short*)(dma_buf+port) = data;
}
void myouti(unsigned int data,unsigned short int port) {
    *(volatile unsigned int*)(dma_buf+port) = data;
}
unsigned char myinc(unsigned short int port) {
    return *(volatile unsigned char*)(dma_buf+port);
}
unsigned short myins(unsigned short int port) {
    return *(volatile unsigned short*)(dma_buf+port);
}
unsigned int myini(unsigned short int port) {
    return *(volatile unsigned int*)(dma_buf+port);
}


static int drv_open(struct inode* ii, struct file* ff) {
	try_module_get(THIS_MODULE);
	printk("%s:%s(): device open\n", PREFIX_TITLE, __func__);
	return 0;
}
static int drv_release(struct inode* ii, struct file* ff) {
	module_put(THIS_MODULE);
	printk("%s:%s(): device close\n", PREFIX_TITLE, __func__);
	return 0;
}

static ssize_t drv_read(struct file *filp, char __user *buffer, size_t ss, loff_t* lo) {
	/* Implement read operation for your device */
	// We just copy the computed result from the device to the user.
    put_user(myini(DMAANSADDR), (int * ) buffer);
    printk("%s:%s(): ans = %d\n", PREFIX_TITLE, __func__, myini(DMAANSADDR));
    // Clean the result.
    myouti(0, DMAANSADDR);
    // Change readable to 0
    myouti(0, DMAREADABLEADDR);
	return 0;
}

//This buffer is got from the user.
static ssize_t drv_write(struct file *filp, const char __user *buffer, size_t ss, loff_t* lo) {

/* Implement write operation for your device */
	unsigned int block;
	unsigned char operator;
	unsigned int operand1;
	unsigned int operand2;
	struct DataIn * tempdata;
    //Start writing process (calculation), set readable signal to 0 until the calculation is finished.
    myouti(0, DMAREADABLEADDR);

    //	Save the data into DMA
    dataIn = kzalloc(sizeof(typeof(struct DataIn)),GFP_KERNEL);
    tempdata = kzalloc(sizeof(typeof(struct DataIn)), GFP_KERNEL);
    copy_from_user(tempdata, buffer, ss);
    operator = dataIn->a = tempdata->a;
    operand1 = dataIn->b = tempdata->b;
    operand2 = dataIn->c = tempdata->c;
    myoutc(operator, DMAOPCODEADDR);
    myouti(operand1, DMAOPERANDBADDR);
    myouti(operand2, DMAOPERANDCADDR);
    //Free tempdata:
    kfree(tempdata);
    //Initialize workload:
    INIT_WORK(work, drv_arithmetic_routine);

    //Check the block / non-block status
    block = myini(DMABLOCKADDR);
    if(block){
        printk("%s:%s(): Queue Work\n", PREFIX_TITLE, __func__ );
        printk("%s:%s(): Block\n", PREFIX_TITLE, __func__ );
        schedule_work(work);
        flush_scheduled_work();
        myouti(1, DMAREADABLEADDR);
    }
    else{
        printk("%s:%s(): Queue Work\n", PREFIX_TITLE, __func__ );
        schedule_work(work);
    }
	return 0;
}
//This arg is an address from the user space. Cannot use it directly in the kernel mode
static long drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	/* Implement ioctl setting for your device */
	unsigned int value_int;
	//Set status information:
	switch(cmd){
	    case HW5_IOCSETSTUID:
	        //Save Stuid into DMA:
            get_user(value_int, (int *) arg);
	        printk("%s:%s(): My STUID is = %d\n", PREFIX_TITLE, __func__ , value_int);
	        myouti(value_int, DMASTUIDADDR);
	        return 1;
	    case HW5_IOCSETRWOK:
            printk("%s:%s(): RW OK\n", PREFIX_TITLE, __func__ );
            myouti(1, DMARWOKADDR);
            return 1;
	    case HW5_IOCSETIOCOK:
            printk("%s:%s(): ioctl OK\n", PREFIX_TITLE, __func__ );
            myouti(1, DMAIOCOKADDR);
            return 1;
	    case HW5_IOCSETIRQOK:
	        myouti(0xFFFFFFFF, DMAIRQOKADDR);
	        return 1;
	    case HW5_IOCSETBLOCK:
	        get_user(value_int, (int * ) arg);
	        //Set blocking:
            if(value_int == 1){
//                put_user(value_int, (int *)arg);
                myouti(value_int, DMABLOCKADDR);
                printk("%s:%s(): Blocking IO\n", PREFIX_TITLE, __func__ );
                return 1;
            }
            //Set non-blocking:
            else if (value_int == 0){
//                put_user(value_int, (int *)arg);
                myouti(value_int, DMABLOCKADDR);
                printk("%s:%s(): Non-Blocking IO\n", PREFIX_TITLE, __func__ );
                return 1;
            }
            else {
                return -1;
            }
	    case HW5_IOCWAITREADABLE:
            /*This method synchronizes the I/O with the application.*/
            while(myini(DMAREADABLEADDR) != 1){
                msleep(1000);
            }
            // Change the value of readable in the user space:
            put_user(1, (int *) arg);
            printk("%s:%s(): wait readable 1\n", PREFIX_TITLE, __func__ );
            return 1;
    }
	return 0;
}

//This function will be executed in another kernel thread.
static void drv_arithmetic_routine(struct work_struct* ws) {
	/* Implement arithmetic routine */
    char operator = dataIn->a;
    int operand1 = dataIn->b;
    short operand2 = dataIn->c;
    int ans;
    //Arithmetic Operations
    switch(operator){
        case '+':
            ans = operand1 + operand2;
            break;
        case '-':
            ans = operand1 - operand2;
            break;
        case '*':
            ans = operand1 * operand2;
            break;
        case '/':
            ans = operand1 / operand2;
            break;
        case 'p':
            ans = prime(operand1, operand2);
            break;
        default:
            ans = -1;
            break;
    }
    //Save answer back to DMA
    myouti(ans, DMAANSADDR);
    //Print Information
    printk("%s:%s(): %d %c %d = %d\n", PREFIX_TITLE, __func__ , operand1, operator, operand2, ans);
    //After the calculation, we set the interrupt signal (readable variable) to 1 (in DMA):
    myouti(1, DMAREADABLEADDR);

}

static int __init init_modules(void) {
    dev_t dev;
	int ret = 0;
    printk("%s:%s():...............Start...............\n", PREFIX_TITLE, __func__);
    ret = alloc_chrdev_region(&dev, 0, 1, "mydev");
	//Register a char device and create the corresponding file system node
	if(ret){
	    printk("%s:%s():Cannot Open the Character Device!\n", PREFIX_TITLE, __func__);
	    return ret;
	}
    dev_major = MAJOR(dev);
    dev_minor = MINOR(dev);
    printk("%s:%s():register chrdev(%d,%d)\n",PREFIX_TITLE, __func__,dev_major,dev_minor);

	/* Init cdev and make it alive */
    dev_cdevp = cdev_alloc();
    cdev_init(dev_cdevp, &drv_fops);
    dev_cdevp->owner = THIS_MODULE;
    dev_cdevp->ops = &drv_fops;
    ret = cdev_add(dev_cdevp, MKDEV(dev_major, dev_minor), 1);
    if(ret < 0){
        printk("%s:%s():Cannot add the Character Device to the system!\n", PREFIX_TITLE, __func__);
    }
	/* Allocate DMA buffer */
    dma_buf = kzalloc(DMA_BUFSIZE, GFP_KERNEL);
    printk("%s:%s():allocate dma buffer\n",PREFIX_TITLE, __FUNCTION__);	/* Allocate work routine */
    work = kmalloc(sizeof(typeof(* work)), GFP_KERNEL);
	return 0;
}

static void __exit exit_modules(void) {
    dev_t dev;
    dev = MKDEV(dev_major, dev_minor);
	/* Free DMA buffer when exit modules */
    kfree(dma_buf);
    printk("%s:%s(): Free DMA Buffer\n", PREFIX_TITLE, __func__);
    // Free dataIn structure
    kfree(dataIn);
	/* Delete character device */
    unregister_chrdev_region(dev, 1);
    printk("%s:%s():Unregister chrdev\n", PREFIX_TITLE, __func__);
    /* Free work routine */


	printk("%s:%s():..............End..............\n", PREFIX_TITLE, __func__);
}


static int prime(int base, short nth)
{
    int fnd=0;
    int i, num, isPrime;

    num = base;
    while(fnd != nth) {
        isPrime=1;
        num++;
        for(i=2;i<=num/2;i++) {
            if(num%i == 0) {
                isPrime=0;
                break;
            }
        }

        if(isPrime) {
            fnd++;
        }
    }
    return num;
}



module_init(init_modules);
module_exit(exit_modules);



