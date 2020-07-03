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

#define BUZZER_MAJOR_NUMBER    502
#define BUZZER_DEV_NAME		"buzzer"

#define GPIO_BASE_ADDR 0x3F200000
#define DMA0_BASE_ADDR 0x3F007000
#define GPFSEL1 0x04
#define GPFSEL2 0x08
#define GPSET0  0x1C
#define GPCLR0  0x28
#define GPLEV0	0x34
#define DMA5	0x500
#define LOW		0
#define HIGH	1

static void __iomem *gpio_base;
static void __iomem *dma0_base;

volatile unsigned int *gpsel1;
volatile unsigned int *gpsel2;
volatile unsigned int *gpset0;
volatile unsigned int *gpclr0;
volatile unsigned int *gplev0;

struct timeval start, end;
unsigned long local_time;

int buzzer_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Buzzer driver open!!\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
	dma0_base = ioremap(DMA0_BASE_ADDR, 0x60);
	
	gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
	gpset0 = (volatile unsigned int *)(gpio_base + GPSET0);
	gpclr0 = (volatile unsigned int *)(gpio_base + GPCLR0);
	
	*gpsel1 |= (1<<24); // if 1<<25, -> GPIO 18 -> alt function 5
	
	return 0;
}

int buzzer_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Servo driver closed!!\n");
	iounmap((void *)gpio_base);
	iounmap((void *)dma0_base);
	return 0;
}


ssize_t buzzer_write(struct file *filp, const char *buf, size_t size, loff_t *f_pos){
	
	int i = 0;
	int value;
	copy_from_user(&value, buf, 4);


	printk(KERN_INFO "%d is value\n",value);
	
	if(value % 2 == 0){
		printk(KERN_ALERT "Right on!!\n");
		for(i=0; i<100; i++){
			*gpset0 |= (1 << 18);
			mdelay(0.05);	//0.001
			*gpclr0 |= (1 << 18);
			mdelay(1.95);
			
		}
	}
	/*
	msleep(0.1);
	
	for(i=0; i<100; i++){
		*gpset0 |= (1 << 18);
		mdelay(0.5);	
		*gpclr0 |= (1 << 18); 
		mdelay(1.5);
	}
	msleep(0.1);
	*/
	for(i=0; i<10; i++){
		*gpset0 |= (1 << 18);
		mdelay(2);	
		*gpclr0 |= (1 << 18);
		mdelay(2);
	}
}




static struct file_operations buzzer_fops= {
	.owner = THIS_MODULE,
	//.read = servo_read,
	.write = buzzer_write,
	.open = buzzer_open,
	.release = buzzer_release
};

int __init buzzer_init(void){
	if(register_chrdev(BUZZER_MAJOR_NUMBER, BUZZER_DEV_NAME, &buzzer_fops) < 0)
		printk(KERN_ALERT "Buzzer driver initialization fail\n");
	else
		printk(KERN_ALERT "Buzzer driver initialization success\n");
		
	return 0;
}

void __exit buzzer_exit(void){
	unregister_chrdev(BUZZER_MAJOR_NUMBER, BUZZER_DEV_NAME);
	printk(KERN_ALERT "Buzzer driver exit done\n");
}

module_init(buzzer_init);
module_exit(buzzer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YouNoGo");
MODULE_DESCRIPTION("des");
