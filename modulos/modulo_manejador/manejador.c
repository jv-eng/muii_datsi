#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

MODULE_AUTHOR("Fernando Pérez Costoya");
MODULE_DESCRIPTION("Manejador básico");
MODULE_LICENSE("GPL");

static struct cdev dispo_desc;
static struct class *dispo_clase;

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
        printk("write %ld bytes\n", count);
        return count;
}
static ssize_t leer(struct file *filp, char __user *buf,
        size_t count, loff_t *f_pos) {
        printk("read %ld bytes\n", count);
        return 0;
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
        if (alloc_chrdev_region(&devnum, minor, 1, "dispo")<0) {
                printk(KERN_ALERT "Error in alloc_chrdev_region\n");
                return -1;
        }
	// crea el dispositivo dentro del núcleo especificando sus operaciones
        cdev_init(&dispo_desc, &dispo_ops);
        if (cdev_add(&dispo_desc, devnum, 1)<0) {
                printk(KERN_ALERT "Error in cdev_add\n");
                return -1;
        }
	// da acceso al dispositivo para los programas
        if ((dispo_clase = class_create(THIS_MODULE, "clase_dispo")) == NULL){
                printk(KERN_ALERT "Error in class_create\n");
                unregister_chrdev_region(devnum, 1);
                return -1;
        }
        device_create(dispo_clase, NULL, devnum, NULL, "dispo");
	return 0;
}
static void __exit manejador_exit(void) {
	printk("Fin de manejador\n");
        device_destroy(dispo_clase, dispo_desc.dev);
        class_destroy(dispo_clase);
        cdev_del(&dispo_desc);
        unregister_chrdev_region(dispo_desc.dev, 1);
}
module_init(manejador_init);
module_exit(manejador_exit); 
