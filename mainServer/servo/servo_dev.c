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

#define SERVO_MAJOR_NUMBER    501
#define SERVO_DEV_NAME		"servo"

#define GPIO_BASE_ADDR 0x3F200000
#define GPFSEL1 0x04
#define GPFSEL2 0x08
#define GPSET0  0x1C
#define GPCLR0  0x28
#define GPLEV0	0x34


static void __iomem *gpio_base;

volatile unsigned int *gpsel1;
volatile unsigned int *gpsel2;
volatile unsigned int *gpset0;
volatile unsigned int *gpclr0;
volatile unsigned int *gplev0;


struct timeval start, end;
unsigned long local_time;

int servo_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Servo driver open!!\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
	
	gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
	gpset0 = (volatile unsigned int *)(gpio_base + GPSET0);
	gpclr0 = (volatile unsigned int *)(gpio_base + GPCLR0);
	
	*gpsel1 |= (1<<24);
	
	return 0;
}

int servo_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Servo driver closed!!\n");
	iounmap((void *)gpio_base);
	return 0;
}


ssize_t servo_write(struct file *filp, const char *buf, size_t size, loff_t *f_pos){
	
	int i = 0;
	char push;
	copy_from_user(&push, buf, sizeof(char));

	printk(KERN_INFO "%c is copied\n",push);
	
	if(push == 'p'){
		printk(KERN_ALERT "Push\n");
		for(i=0; i<100; i++){
			*gpset0 |= (1 << 18);
			mdelay(0.5);
			*gpclr0 |= (1 << 18);
			mdelay(19.5);
			
		}
	}
    ssleep(1);
	printk(KERN_INFO "To original position\n");
	for(i=0; i<100; i++)
	{
		*gpset0 |= (1 << 18);
		mdelay(2.5);	
		*gpclr0 |= (1 << 18);
		mdelay(17.5);
	}

	return size;
}




static struct file_operations servo_fops= {
	.owner = THIS_MODULE,
	//.read = servo_read,
	.write = servo_write,
	.open = servo_open,
	.release = servo_release
};

int __init servo_init(void){
	if(register_chrdev(SERVO_MAJOR_NUMBER, SERVO_DEV_NAME, &servo_fops) < 0)
		printk(KERN_ALERT "Servo driver initialization fail\n");
	else
		printk(KERN_ALERT "Servo driver initialization success\n");
		
	return 0;
}

void __exit servo_exit(void){
	unregister_chrdev(SERVO_MAJOR_NUMBER, SERVO_DEV_NAME);
	printk(KERN_ALERT "Servo driver exit done\n");
}

module_init(servo_init);
module_exit(servo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YouNoGo");
MODULE_DESCRIPTION("des");
