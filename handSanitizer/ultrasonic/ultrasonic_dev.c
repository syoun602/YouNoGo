#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/time.h>

#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define ULT_MAJOR_NUMBER    505 // ULT = ultrasonic
#define ULT_DEV_NAME		"ultrasonic"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0x04
#define GPFSEL2 0x08
#define GPSET0  0x1C
#define GPCLR0  0x28
#define GPLEV0	0x34

static void __iomem *gpio_base;
volatile unsigned int *gpsel2;
volatile unsigned int *gpset0;
volatile unsigned int *gpclr0;
volatile unsigned int *gplev0;

struct timeval start, end;
unsigned long local_time;

int ultrasonic_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Ultrasonic driver open!!\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
	
	gpsel2 = (volatile unsigned int *)(gpio_base + GPFSEL2);
	gplev0 = (volatile unsigned int *)(gpio_base + GPLEV0);
	gpset0 = (volatile unsigned int *)(gpio_base + GPSET0);
	gpclr0 = (volatile unsigned int *)(gpio_base + GPCLR0);
	
	*gpsel2 |= (1<<3); //GPIO 21 -> output mode = trig
	//*gpsel2 |= (0<<6); //GPIO 20 -> input mode  = echo
	
	return 0;
}

int ultrasonic_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "ultrasonic driver closed!!\n");
	iounmap((void *)gpio_base);
	return 0;
}

ssize_t ultrasonic_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

	int val = 0;
	printk(KERN_ALERT"ultrasonic device read function called\n");	
	
	*gpclr0 = (1 << 21); // trig -> LOW	
	ssleep(0.5);	
	*gpset0 = (1 << 21); // trig -> HIGH for 10 us
	ssleep(0.00001);
	*gpclr0 = (1 << 21);
	
	while((*gplev0 & (1 << 20)) == 0) {
		do_gettimeofday(&start);  // get start time
	}
	//printk("%lu\n", start.tv_sec);
	//printk("%lu\n", start.tv_usec);
	
	while((*gplev0 & (1 << 20)) > 0) {
		do_gettimeofday(&end);
	}
	//printk("%lu\n", end.tv_sec);
	//printk("%lu\n", end.tv_usec);
	
	val = end.tv_usec - start.tv_usec; //
	
	if((end.tv_sec - start.tv_sec) == 0)
		val = val;
	else
		val = val + (end.tv_sec - start.tv_sec) * 1000000; // usec + sec*1000000
		
	//val = val/58;
	printk("%d\n", val/58);

		
	copy_to_user(buf, &val, sizeof(int)); 
    
	return count; 
}

static struct file_operations ultrasonic_fops= {
	.owner = THIS_MODULE,
	.read = ultrasonic_read,
	.open = ultrasonic_open,
	.release = ultrasonic_release
};

int __init ultrasonic_init(void){
	if(register_chrdev(ULT_MAJOR_NUMBER, ULT_DEV_NAME, &ultrasonic_fops) < 0)
		printk(KERN_ALERT "Ultrasonic driver initialization fail\n");
	else
		printk(KERN_ALERT "Ultrasonic driver initialization success\n");
		
	return 0;
}

void __exit ultrasonic_exit(void){
	unregister_chrdev(ULT_MAJOR_NUMBER, ULT_DEV_NAME);
	printk(KERN_ALERT "Ultrasonic driver exit done\n");
}

module_init(ultrasonic_init);
module_exit(ultrasonic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sun");
MODULE_DESCRIPTION("des");
