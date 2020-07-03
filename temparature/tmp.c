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

#define TMP_MAJOR_NUMBER    504 //
#define TMP_DEV_NAME		"tmp"

#define GPIO_BASE_ADDR 0x3F200000
#define BSC1_BASE_ADDR 0x3F804000

#define BSC_A	0x0C //master slave address
#define BSC_C	0x00
#define BSC_S	0x04
#define BSC_DLEN 0x08
#define BSC_FIFO 0x10
#define BSC_DEL  0x18

#define RW 0b00000010
#define EN 0b00000100

#define BSC_C_I2CEN 0x00008000
#define BSC_C_ST	0x00000080
#define BSC_C_READ 	0x00000001
#define BSC_C_CLEAR 0x00000020

#define BSC_S_TXD	0x00000010
#define BSC_S_RXD	0x00000020
#define BSC_S_ERR 	0x00000100
#define BSC_S_DONE	0x00000002
#define BSC_S_CLKT 	0x00000200

#define GPFSEL0 0x00
#define GPSET0  0x1C
#define GPCLR0  0x28

static void __iomem *gpio_base;
static void __iomem *bsc1_base;

volatile unsigned int *gpsel0;
volatile unsigned int *gpsel1;
volatile unsigned int *gpsel2;
volatile unsigned int *gpset0;

volatile unsigned int *bsc_a;
volatile unsigned int *bsc_dlen;
volatile unsigned int *bsc_fifo;
volatile unsigned int *bsc_status;
volatile unsigned int *bsc_control;

void i2c_write(const char* buf, unsigned int len);
void tmp_send_byte(const unsigned char byte, const unsigned char mode);

int tmp_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "TMP driver open!!\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
	bsc1_base = ioremap(BSC1_BASE_ADDR, 0x60);

	gpsel0 = (volatile unsigned int *)(gpio_base + GPFSEL0);
	bsc_a = (volatile unsigned int *)(bsc1_base + BSC_A);
	
	bsc_dlen = (volatile unsigned int *)(bsc1_base + BSC_DLEN);
	bsc_fifo = (volatile unsigned int *)(bsc1_base + BSC_FIFO);
	bsc_status = (volatile unsigned int *)(bsc1_base + BSC_S);
	bsc_control = (volatile unsigned int *)(bsc1_base + BSC_C);
	
	*bsc_a |= (0x40); // Slave Address
	
	return 0;
}

int tmp_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "TMP driver closed!!\n");
	iounmap((void *)gpio_base);
	iounmap((void *)bsc1_base);
	return 0;
}

void i2c_write(const char* buf, unsigned int len){
	
	unsigned int remaining = len;
	unsigned int i = 0;
	
	//Clear FIFO
	*bsc_control = (*bsc_control & ~BSC_C_CLEAR) | BSC_C_CLEAR;
	
	// Clear Status
	*bsc_status = BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE;
	
	//Set Data Length
	*bsc_dlen = len;
	
	// Enable I2C and start transfer
	*bsc_control = BSC_C_I2CEN | BSC_C_ST;
	while( remaining && (i<16))
	{
		*bsc_fifo = buf[i];
		//printk(KERN_ALERT "%c  in i2c write buf[%d]\n", buf[i], i);
		//printk(KERN_ALERT "%d  in i2c write i\n", i);
		//printk(KERN_ALERT "%d  in i2c write remaining", remaining);
		i++;
		remaining--;		
	}
	
	*bsc_control = (*bsc_control & ~BSC_S_DONE) | BSC_S_DONE;
}

ssize_t tmp_write(struct file *filp, const char *buf, size_t size, loff_t *f_pos){

	// 0x02 -> Configuration register
	//0x15,0x40 ->  Continuous conversion, comparator mode
	char config[3] = {0x02,0x15,0x40};
	char reg[1];
	copy_from_user(reg, buf, 1);
	printk(KERN_ALERT "%s : input \n", reg);
	
	i2c_write(&config[0], 1);	
	ssleep(0.0001);
	i2c_write(&config[1], 1);	
	ssleep(0.0001);
	i2c_write(&config[2], 1);	
	ssleep(0.0001);
	i2c_write(reg, 1); // temp result register 0x03	
	ssleep(0.0001);
	
	ssleep(1);
}

ssize_t tmp_read(struct file *filp, char *buf, size_t size, loff_t *f_pos){
	printk(KERN_ALERT "tmp_read function called\n");
	
	unsigned int remaining = size;
	unsigned int i = 0;
	
	//Clear FIFO
	*bsc_control = (*bsc_control & ~BSC_C_CLEAR) | BSC_C_CLEAR;
	
	// Clear Status
	*bsc_status = BSC_S_CLKT | BSC_S_ERR | BSC_S_DONE;
	
	//Set Data Length
	*bsc_dlen = size;
	
	//Start read
	*bsc_control = BSC_C_I2CEN | BSC_C_ST | BSC_C_READ;
	//wait for transfer to complete
	while(!(*bsc_status & BSC_S_DONE)){
		// we must empty the FIFO as it is populated and not use any delay
		while(*bsc_status & BSC_S_RXD){
			// Read from FIFO
			buf[i] = *bsc_fifo;
			i++;
			remaining--;
		}
	}
	
	*bsc_control = (*bsc_control & BSC_S_DONE) | BSC_S_DONE;
	ssleep(1);
	return size;
}

static struct file_operations tmp_fops= {
	.owner = THIS_MODULE,
	.read = tmp_read,
	.write = tmp_write,
	.open = tmp_open,
	.release = tmp_release
};

int __init tmp_init(void){
	if(register_chrdev(TMP_MAJOR_NUMBER, TMP_DEV_NAME, &tmp_fops) < 0)
		printk(KERN_ALERT "TMP driver initialization fail\n");
	else
		printk(KERN_ALERT "TMP driver initialization success\n");
		
	return 0;
}

void __exit tmp_exit(void){
	unregister_chrdev(TMP_MAJOR_NUMBER, TMP_DEV_NAME);
	printk(KERN_ALERT "TMP driver exit done\n");
}

module_init(tmp_init);
module_exit(tmp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YouNoGo");
MODULE_DESCRIPTION("des");

