#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/kfifo.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>

//constantes
#define DEVICE_NAME "int_spkr"

//datos del modulo
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Juan Alfonso Viejo Rodriguez <juanalfonso.viejo.rodriguez@alumnos.upm.es>");
MODULE_DESCRIPTION("sound driver for pc");
MODULE_VERSION("0.0.1");


//declaracion de funciones
extern void set_spkr_frequency(unsigned int frequency);
extern void spkr_on(void);
extern void spkr_off(void);

//variables globales
static int frecuencia = 0; module_param(frecuencia, int, S_IRUGO);

//crear dispositivo
static dev_t midispo;
int spkr_minor = 0; module_param(spkr_minor, int, S_IRUGO);
static struct cdev c_dev;
static struct class *cl;


/////////////////////////////////////////////////////////////////////////
/*Coidgo*/
/////////////////////////////////////////////////////////////////////////

//device open
static int device_open(struct inode *inode, struct file *filp) {
    printk("device_open\n");
    return 0;
}

//device release
static int device_release(struct inode *inode, struct file *file) {
    printk("device_release\n");
    return 0;
}

//device write
static ssize_t device_write(struct file *filp, const char *buff, size_t count, loff_t *f_pos) {
    printk("device_write\n");
    return 0;
}

//definicion de funciones del driver
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .write = device_write
};

//inicio del driver
static int __init init_initpkr(void) {
    //variables locales
    
    //crear dispositivo
    if (alloc_chrdev_region(&midispo, spkr_minor, 1, DEVICE_NAME) < 0) {
		printk(KERN_ALERT "Device Registration failed\n");
		return -1;
	}
    cdev_init(&c_dev, &fops); //iniciacion del modulo
	//alta del dispositivo
	if( cdev_add( &c_dev, midispo, 1 ) == -1) {
		printk( KERN_ALERT "Device addition failed\n" );
		device_destroy( cl, midispo );
		class_destroy( cl );
		unregister_chrdev_region( midispo, 1 );
		return -1;
	}
    //crear clase en sysfs
    if ((cl = class_create(THIS_MODULE, "speaker")) == NULL) {
		printk(KERN_ALERT "Class creation failed\n");
		unregister_chrdev_region(midispo, 1);
		return -1;
	}
	//crear clase
	if(device_create(cl, NULL, midispo, NULL, DEVICE_NAME) == NULL) {
		printk(KERN_ALERT "Device creation failed\n");
		class_destroy(cl);
		unregister_chrdev_region(midispo, 1);
		return -1;
	}

    printk("%d %d\n", midispo, spkr_minor);

    set_spkr_frequency(frecuencia);
    spkr_on();
    

    printk("Inicio del driver\n");
    return 0;
}

//fin del driver
static void __exit exit_intpkr(void) {
    //variables locales

    //liberar dispositivo
    unregister_chrdev_region(midispo, 1);
    cdev_del(&c_dev);
    device_destroy(cl, midispo);
    class_destroy(cl);
    
    
    printk("fin del modulo\n");
    spkr_off();
}


module_init(init_initpkr);
module_exit(exit_intpkr);