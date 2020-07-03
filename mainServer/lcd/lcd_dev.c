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

#define LCD_MAJOR_NUMBER    503 //
#define LCD_DEV_NAME		"lcd"


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

#define LCD_BACKLIGHT 0b00001000
#define LCD_LINE_1  0x80
#define LCD_LINE_2  0xC0

#define BSC_C_I2CEN 0x00008000
#define BSC_C_ST	0x00000080
#define BSC_C_CLEAR 0x00000020

#define BSC_S_TXD	0x00000010
#define BSC_S_ERR 	0x00000100
#define BSC_S_DONE	0x00000002
#define BSC_S_CLKT	0x00000200

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



void lcd_send_byte(const unsigned char byte, const unsigned char mode);
void i2c_write(const char* buf, unsigned int len);

int lcd_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "LCD driver open!!\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
	bsc1_base = ioremap(BSC1_BASE_ADDR, 0x60);

	gpsel0 = (volatile unsigned int *)(gpio_base + GPFSEL0);
	bsc_a = (volatile unsigned int *)(bsc1_base + BSC_A);
	bsc_dlen = (volatile unsigned int *)(bsc1_base + BSC_DLEN);
	bsc_fifo = (volatile unsigned int *)(bsc1_base + BSC_FIFO);
	bsc_status = (volatile unsigned int *)(bsc1_base + BSC_S);
	bsc_control = (volatile unsigned int *)(bsc1_base + BSC_C);
	
	*bsc_a |= (0x3f); // Master Slave Address
	
	return 0;
}

	
int lcd_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "LCD driver closed!!\n");
	iounmap((void *)gpio_base);
	iounmap((void *)bsc1_base);
	return 0;
}

void i2c_write(const char* buf, unsigned int len){
	unsigned int remaining = len;
	unsigned int i = 0;
	unsigned char reason = 0x00;	
	unsigned int v;
	int count = 0;
	int count2 = 0;
	
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

void lcd_send_byte(const unsigned char byte, unsigned char mode )
{
	unsigned char high_nibble, low_nibble;
	unsigned char buf1, buf2;

	high_nibble = (mode) | (byte & 0xF0) | (LCD_BACKLIGHT & (~RW));
	low_nibble = mode | ((byte << 4) & 0xF0) | (LCD_BACKLIGHT & (~RW));
	
	buf1 = high_nibble | EN;
	ssleep(0.00001);
	i2c_write((char*)&buf1, 1);
	ssleep(0.000001); 
	buf1 = high_nibble & ~EN;
	i2c_write((char*)&buf1, 1);
	ssleep(0.00001);
	ssleep(0.0001); 
	
	
	buf2 = low_nibble | EN;
	ssleep(0.00001);
	i2c_write((char*)&buf2, 1);
	ssleep(0.000001);
	buf2 = low_nibble & ~EN;
	i2c_write((char*)&buf2, 1);
	ssleep(0.00001);	
	ssleep(0.0001);
}
ssize_t lcd_write(struct file *filp, const char *buf, size_t size, loff_t *f_pos){
	
	int i = 1, remaining = 1, ret;
	int v;
	char str[1024];
	char *grant = "Access Granted";
	char *deny = "Access Denied";

	unsigned char pos;

	copy_from_user(str, buf, 1024);
	printk(KERN_ALERT "%s : input \n", str);
	
	// backlight, cursor, ... Initialize
	lcd_send_byte(0b00110011, 0);	// Initialize
	ssleep(0.00001);
	lcd_send_byte(0b00110010, 0);	// Initialize
	ssleep(0.00001);
	lcd_send_byte(0b00000110, 0);	// Cursor move direction
	ssleep(0.00001);
	lcd_send_byte(0b00001100, 0);	// Display On, Cursor Off, Blink Off
	ssleep(0.00001);
	lcd_send_byte(0b00101000, 0);	// Data length, number of lines, font size
	ssleep(0.00001);
	lcd_send_byte(0b00000001, 0);	// Clear screen
	ssleep(0.00001);
	
	
	printk(KERN_INFO "%d\n",str[0]);
	
	i = 1;
	//LCD first line Initialize
	lcd_send_byte(LCD_LINE_1, 0);
	while(strlen(str) != i)
	{
		lcd_send_byte(str[i], 1);
		i++;
	}
	
	i = 0;
	if(str[0] == '1'){
		//LCD second line Initialize
		lcd_send_byte(LCD_LINE_2, 0);
		while(strlen(grant) != i)
		{
			lcd_send_byte(grant[i], 1);
			i++;
		}
	}
	else if(str[0] == '0'){
		//LCD second line Initialize
		lcd_send_byte(LCD_LINE_2, 0);
		while(strlen(deny) != i)
		{
			lcd_send_byte(deny[i], 1);
			i++;
		}
	}

	
	printk(KERN_ALERT "Displayed on LCD\n");
	ssleep(1);
}




static struct file_operations lcd_fops= {
	.owner = THIS_MODULE,
	//.read = lcd_read,
	.write = lcd_write,
	.open = lcd_open,
	.release = lcd_release
};

int __init lcd_init(void){
	if(register_chrdev(LCD_MAJOR_NUMBER, LCD_DEV_NAME, &lcd_fops) < 0)
		printk(KERN_ALERT "LCD driver initialization fail\n");
	else
		printk(KERN_ALERT "LCD driver initialization success\n");
		
	return 0;
}

void __exit lcd_exit(void){
	unregister_chrdev(LCD_MAJOR_NUMBER, LCD_DEV_NAME);
	printk(KERN_ALERT "LCD driver exit done\n");
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YouNoGo");
MODULE_DESCRIPTION("des");
