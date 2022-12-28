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

/////////////////////////////////////////////////////////////////////////
/*Variables globales*/
/////////////////////////////////////////////////////////////////////////

//declaracion de funciones
extern void set_spkr_frequency(unsigned int frequency);
extern void spkr_on(void);
extern void spkr_off(void);

//variables globales
static int frecuencia = 0; module_param(frecuencia, int, S_IRUGO);
static int device_open_cont = 0;	//ver si el dispositivo esta en uso

//mutex
struct mutex open_device_mutex;
struct mutex write_device_mutex;

//spinlock
spinlock_t lock_write;

//gestion de procesos
static wait_queue_head_t cola;
static struct kfifo fifo;

//gestion de temporizadores
static struct timer_list timer;

//crear dispositivo
static dev_t midispo;
int spkr_minor = 0; module_param(spkr_minor, int, S_IRUGO);
static struct cdev c_dev;
static struct class *cl;


/////////////////////////////////////////////////////////////////////////
/*Coidgo*/
/////////////////////////////////////////////////////////////////////////

void int_temp(struct timer_list *t) {
    printk("int_temp\n");
}

void add_timer_(long time) {
    //comprobar si queda tiempo
    if (timer_pending(&timer)){
		printk("timer pending\n");
		return;
	}

    //configuramos el temporizador
	timer.function = int_temp;
	timer.expires = jiffies + msecs_to_jiffies(time); 

    printk("timer added\n");

	add_timer(&timer);
}

//device open
static int device_open(struct inode *inode, struct file *filp) {
    printk("device_open\n");

    //diferenciamos lectura y escritura
    if (filp->f_mode & FMODE_READ) {
        printk("device_open - dispositivo abierto en modo lectura\n");
    } else if (filp->f_mode & FMODE_WRITE) { 
        mutex_lock(&open_device_mutex);
        if (device_open_cont) {
            printk("device_open - el dispositivo esta en uso\n");
            mutex_unlock(&open_device_mutex);
            return -EBUSY;
        }
        device_open_cont++;
        mutex_unlock(&open_device_mutex);
        printk("device_open - dispositivo abierto en modo escritura\n");
    }

    return 0;
}

//device release
static int device_release(struct inode *inode, struct file *file) {
    //liberar mutex escritura
    if (file->f_mode & FMODE_WRITE) {
        mutex_lock(&open_device_mutex);
        device_open_cont--;
        mutex_unlock(&open_device_mutex);
    }
    
    printk("device_release\n");
    return 0;
}

//device write
static ssize_t device_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    //variables locales
    char local_buff = '0';
    int elem_write = count, i;

    

    //check count
    if (count < 0) {
        mutex_unlock(&write_device_mutex);
        return -EFAULT;
    }

    //check memoria de usuario
    if (get_user(local_buff, buf) != 0) {
        mutex_unlock(&write_device_mutex);
        return -EFAULT;
    }

    //inicio seccion critica, lectura de sonidos
    spin_lock(&lock_write);
    for (i = 0; i < count; i++) {
        
        if (kfifo_avail(&fifo) == 0) {
            spin_unlock(&lock_write);

            add_timer_(200);

            if(wait_event_interruptible(cola, kfifo_avail(&fifo) >0)) {
                return -ERESTARTSYS;
            }

            spin_lock(&lock_write);
        }

        kfifo_put(&fifo, buf[i]);
        elem_write--;
    }

    add_timer_(200);
    spin_unlock(&lock_write);


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
    //crear dispositivo
    if (alloc_chrdev_region(&midispo, spkr_minor, 1, DEVICE_NAME) < 0) {
		printk("fallo al registrar al dispositivo\n");
		return -1;
	}
    cdev_init(&c_dev, &fops); //iniciacion del modulo
	//alta del dispositivo
	if( cdev_add( &c_dev, midispo, 1 ) == -1) {
		printk("error al añadir el dispositivo\n" );
		device_destroy( cl, midispo );
		class_destroy( cl );
		unregister_chrdev_region( midispo, 1 );
		return -1;
	}
    //crear clase en sysfs
    if ((cl = class_create(THIS_MODULE, "speaker")) == NULL) {
		printk("error al crear la clase\n");
		unregister_chrdev_region(midispo, 1);
		return -1;
	}
	//crear clase
	if(device_create(cl, NULL, midispo, NULL, DEVICE_NAME) == NULL) {
		printk("error al crear el dispositivo\n");
		class_destroy(cl);
		unregister_chrdev_region(midispo, 1);
		return -1;
	}

    //iniciar los mutex
    mutex_init(&open_device_mutex);
    mutex_init(&write_device_mutex);

    //iniciar spinlock
    spin_lock_init(&lock_write);

    //iniciar kfifo
    if (kfifo_alloc(&fifo, 1, GFP_KERNEL)) {
		printk(KERN_WARNING "error kfifo_alloc\n");
		return -ENOMEM;
	}

    //gestionar procesos
    init_waitqueue_head(&cola);

    //inicializar temporizador
    timer_setup(&timer, int_temp, 0); 
    
    //para probar el modulo
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

    //destruir mutex
    mutex_destroy(&open_device_mutex);

    //eliminar temporizador
    del_timer_sync(&timer);
    
    
    printk("fin del modulo\n");
    spkr_off();
}


module_init(init_initpkr);
module_exit(exit_intpkr);