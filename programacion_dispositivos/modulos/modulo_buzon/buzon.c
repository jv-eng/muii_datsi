#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/minmax.h>

MODULE_AUTHOR("Fernando Pérez Costoya");
MODULE_DESCRIPTION("Manejador que implementa mecanismo de comunicación simple");
MODULE_LICENSE("GPL");

static size_t buzon_tam = 4096;
module_param(buzon_tam, ulong, S_IRUSR);
MODULE_PARM_DESC(buzon_tam, "El tamaño del buzón (defecto 4096)");

static struct cdev dispo_desc;
static struct class *dispo_clase;
static struct mutex mutex;
static void * buzon;
static size_t tam=0;
static wait_queue_head_t cola_espera;

static int abrir(struct inode *inode, struct file *filp) {
        printk("open %s\n", filp->f_mode & FMODE_WRITE?"escritura":"lectura");
        return 0;
}
static int cerrar(struct inode *inode, struct file *filp) {
        printk("release\n");
        return 0;
}
static ssize_t escribir(struct file *filp, const char __user *buf,
        size_t count, loff_t *f_pos) {
	int ret;
        printk("write %ld bytes\n", count);
        if (mutex_lock_interruptible(&mutex)) return -ERESTARTSYS;
        if (tam) {
        	mutex_unlock(&mutex);
                return -ENOSPC;
	}
	else {
		tam=min(buzon_tam, count);
		if (copy_from_user(buzon, buf, tam)) ret = -EFAULT;
		wake_up_interruptible(&cola_espera);
	}
        mutex_unlock(&mutex);
        return tam;
}
static ssize_t leer(struct file *filp, char __user *buf,
        size_t count, loff_t *f_pos) {
	size_t copiado;
	int ret;
        printk("read %ld bytes TAM %ld\n", count, tam);
        if (mutex_lock_interruptible(&mutex)) return -ERESTARTSYS;
	if (tam==0) {
        	mutex_unlock(&mutex);
        	if ((ret=wait_event_interruptible(cola_espera, tam))) return ret;
                if (mutex_lock_interruptible(&mutex)) return -ERESTARTSYS;
	}
	copiado=min(tam, count);
	if (copy_to_user(buf, buzon, copiado)) ret = -EFAULT;;
	tam=0;
        mutex_unlock(&mutex);
        return copiado;
}
static struct file_operations dispo_ops = {
        .owner =          THIS_MODULE,
        .open =           abrir,
        .release =        cerrar,
        .write =          escribir,
        .read =           leer,
};
static int __init manejador_init(void) {
        dev_t devnum;
	int minor=0;
	printk("Inicio de manejador de un único dispositivo\n");
	// solicita major para el manejador de un solo dispositivo (minor=0)
        if (alloc_chrdev_region(&devnum, minor, 1, "buzon")<0) {
                printk(KERN_ALERT "Error in alloc_chrdev_region\n");
                return -1;
        }
	// crea el dispositivo dentro del núcleo especificando sus operaciones
        cdev_init(&dispo_desc, &dispo_ops);
        if (cdev_add(&dispo_desc, devnum, 1)<0) {
                printk(KERN_ALERT "Error in cdev_add\n");
                return -1;
        }
	// da acceso al dispositivo 
        if ((dispo_clase = class_create(THIS_MODULE, "almacenamiento")) == NULL){
                printk(KERN_ALERT "Error in class_create\n");
                unregister_chrdev_region(devnum, 1);
                return -1;
        }
        device_create(dispo_clase, NULL, devnum, NULL, "buzon");
        mutex_init(&mutex);
        init_waitqueue_head(&cola_espera);
	if (!(buzon = kmalloc(buzon_tam, GFP_KERNEL)))
		return -ENOMEM;
	return 0;
}
static void __exit manejador_exit(void) {
	printk("Fin de manejador\n");
	kfree(buzon);
        device_destroy(dispo_clase, dispo_desc.dev);
        class_destroy(dispo_clase);
        cdev_del(&dispo_desc);
        unregister_chrdev_region(dispo_desc.dev, 1);

}
module_init(manejador_init);
module_exit(manejador_exit);
