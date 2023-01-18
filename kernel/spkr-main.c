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

void add_timer_(long time);

/////////////////////////////////////////////////////////////////////////
/*Variables globales*/
/////////////////////////////////////////////////////////////////////////

//declaracion de funciones
extern void set_spkr_frequency(unsigned int frequency);
extern void spkr_on(void);
extern void spkr_off(void);

//variables globales
static int device_open_cont = 0;	//ver si el dispositivo esta en uso
static int temp = 0;
int ms_temp = 0;
static uint32_t buffersize = 0; module_param(buffersize, int, S_IRUGO);
int mute[1];

//kfifo
static struct kfifo fifo;

//mutex
struct mutex open_device_mutex;
struct mutex ioctl_mutex;

//spinlock
spinlock_t lock_write;
spinlock_t lock_int_temp;

//gestion de procesos
wait_queue_head_t cola;

//gestion de temporizadores
static struct timer_list timer;
static struct timer_list timer_not_buff;

//crear dispositivo
static dev_t midispo;
int spkr_minor = 0; module_param(spkr_minor, int, S_IRUGO);
static struct cdev c_dev;
static struct class *cl;

//variables de ioctl
#define MAGIC_NO '9'
#define SPKR_SET_MUTE_STATE _IOR(MAGIC_NO, 1, int *) 
#define SPKR_GET_MUTE_STATE _IOR(MAGIC_NO, 2, int *) 


/////////////////////////////////////////////////////////////////////////
/*Coidgo*/
/////////////////////////////////////////////////////////////////////////

void int_temp(struct timer_list *t) {
    //variables locales
    uint16_t freq = 0, ms = 0;
    printk("interrupcion int_temp\n");
    
    //seccion critica
    spin_lock_bh(&lock_int_temp);

    //si hay un 
    if (kfifo_len(&fifo) < 4) {
	    wake_up_interruptible(&cola);
        temp = 0;
        add_timer_(50);
    } else {
        if (kfifo_out(&fifo, &ms, 2) < 0) {return;}
        if (kfifo_out(&fifo, &freq, 2) < 0) {return;}

        //printk("kernel %d\t%d\n", ms, freq);

        if (freq > 0) {
            set_spkr_frequency(freq);
            if (mute[0] == 0) {spkr_on();}
        } else { //desactivar el altavoz si hay frecuencia = 0   
            spkr_off();
        }

        add_timer_(ms);

    }

    if (kfifo_is_empty(&fifo)) {
        spkr_off();
    }

    spin_unlock_bh(&lock_int_temp);
}

void int_temp_not_buf(struct timer_list *t) {
    printk("interrupcion int_temp_not_buf\n");
    
    //seccion critica
    spin_lock_bh(&lock_int_temp);
	wake_up_interruptible(&cola);
    temp = 0;
    spin_unlock_bh(&lock_int_temp);
}

void add_timer_(long time) {
    printk("timer added\n");
    //spkr_off();
	mod_timer(&timer, jiffies + msecs_to_jiffies(time));
}
void add_timer_not_buf(long time) {
    printk("timer added\n");
    //spkr_off();
	mod_timer(&timer_not_buff, jiffies + msecs_to_jiffies(time));
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

static int write_buf(const char __user *buf, size_t count) {
    //variables locales
    int i, cont = count, copied = 0;
        
    printk("device_write\n");

    //inicio seccion critica, lectura de sonidos
    spin_lock_bh(&lock_write);

    for (i = 0; i < count; i += 4) {
        //sonido
        //si no queda espacio, dormimos el proceso
        if (kfifo_avail(&fifo) < 4) {
            add_timer_(50);
            temp = 1;

            spin_unlock_bh(&lock_write);
            if (wait_event_interruptible(cola, kfifo_avail(&fifo) > 0)) {
                spin_unlock_bh(&lock_write);
                return -1;
            }
            spin_lock_bh(&lock_write);
        }
        printk("guardamos sonido\n");
        
         
        //guardamos sonido
        if (kfifo_from_user(&fifo, buf + i, (cont >= 4) ? 4 : cont, &copied) != 0) {
            printk("error al leer los datos del buffer\n");
            return -1;
        }

        spkr_off();
        cont -= 4;
    }
    add_timer_(50);

    spin_unlock_bh(&lock_write);
    return count;
}


static int write_not_buf(const char __user *buf, size_t count) {
    //variables locales
    uint16_t freq, ms;
    int i, cont = 0;
        
    printk("device_write\n");

    //inicio seccion critica, lectura de sonidos
    spin_lock_bh(&lock_write);

    for (i = count; i >= 4; i -= 4) {

        if (get_user(ms, (u_int16_t __user *)buf + cont) != 0) {return -1;}
        if (get_user(freq, (u_int16_t __user *)buf + cont + 1) != 0) {return -1;}
                
        if (freq > 0) {
            set_spkr_frequency(freq);
            if (mute[0] == 0) {spkr_on();}
        } else { //desactivar el altavoz si hay frecuencia = 0
            spkr_off();
        }
        

        add_timer_not_buf(ms);
        temp = 1;

        spin_unlock_bh(&lock_write);
        if (wait_event_interruptible(cola, temp == 0)) {
            spin_unlock_bh(&lock_write);
            return -1;
		}

        spkr_off();
        spin_lock_bh(&lock_write);
        cont += 2;
    }

    //ha quedado algo sin leer
    if (i > 0) {
        if (ms_temp != -1) { //meter sonido

            if (get_user(freq, (u_int16_t __user *)buf + cont) != 0) {return -1;}
            if (freq > 0) {
                set_spkr_frequency(freq);
                if (mute[0] == 0) {spkr_on();}
            } else { //desactivar el altavoz si hay frecuencia = 0
                spkr_off();
            }
            add_timer_not_buf(ms_temp);
            temp = 1;
            spin_unlock_bh(&lock_write);
            if (wait_event_interruptible(cola, temp == 0)) {
                spin_unlock_bh(&lock_write);
                return -1;
            }
            spin_lock_bh(&lock_write);
            spkr_off();
        } else { //guardar dato
            if (get_user(ms_temp, (u_int16_t __user *)buf + cont) != 0) {return -1;}
        }

    }

    spin_unlock_bh(&lock_write);

    return count;
}

//device write
static ssize_t device_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    
    if (buffersize == 0) {
        //caso sin buffer interno
        write_not_buf(buf, count);
    } else {
        //caso con buffer interno
        write_buf(buf,count);
    }

    return count;
}

static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int *temp = mute[0];
    printk("mute\n");
    mutex_lock(&ioctl_mutex);
    
    switch(cmd) {
        case SPKR_SET_MUTE_STATE:
            if (copy_from_user(&temp, (int *) arg, sizeof(int))) {     
				return -EFAULT;
            }
            mute[0] = temp;  
			if (mute[0]) {
				spkr_off();
            }
            break;
        case SPKR_GET_MUTE_STATE:            
            if(copy_to_user((int *)arg, &temp, sizeof(int))) {
				return -EFAULT;
            }
            break;
    }

    mutex_unlock(&ioctl_mutex);
    return 0;
}

//definicion de funciones del driver
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .write = device_write,
    .unlocked_ioctl = device_ioctl
};

//inicio del driver
static int __init init_initpkr(void) {

    //iniciar los mutex
    mutex_init(&open_device_mutex);
    mutex_init(&ioctl_mutex);
    mute[0] = 0;

    //iniciar spinlock
    spin_lock_init(&lock_write);
    spin_lock_init(&lock_int_temp);
    
    //gestionar procesos
    init_waitqueue_head(&cola);

    //inicializar temporizador
    timer_setup(&timer, int_temp, 0);
    timer_setup(&timer_not_buff, int_temp_not_buf, 0);

    //inicializar fifo
	if (buffersize) {
        if (kfifo_alloc(&fifo, buffersize, GFP_KERNEL)) {
            printk(KERN_WARNING "error kfifo_alloc\n");
            return -ENOMEM;
        }
        printk("size del fifo: %u\n", kfifo_size(&fifo));
    }
    

    //crear dispositivo
    if (alloc_chrdev_region(&midispo, spkr_minor, 1, DEVICE_NAME) < 0) {
		printk("fallo al registrar al dispositivo\n");
		return -1;
	}
    cdev_init(&c_dev, &fops); //iniciacion del modulo
	//alta del dispositivo
	if(cdev_add(&c_dev, midispo, 1) == -1) {
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

    printk("disp creado\n");
    printk("Major: %d\n",MAJOR(midispo));

    printk("Inicio del driver\n");
    return 0;
}

//fin del driver
static void __exit exit_intpkr(void) {
    //liberar dispositivo
    unregister_chrdev_region(midispo, 1);
    cdev_del(&c_dev);
    device_destroy(cl, midispo);
    class_destroy(cl);

    //destruir mutex
    mutex_destroy(&open_device_mutex);

    //destruir fifo
    kfifo_free(&fifo);

    //eliminar temporizador
    del_timer_sync(&timer);
    del_timer_sync(&timer_not_buff);
    
    printk("fin del modulo\n");
    spkr_off();
}


module_init(init_initpkr);
module_exit(exit_intpkr);