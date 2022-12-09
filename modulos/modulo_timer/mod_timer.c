#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

MODULE_AUTHOR("Fernando Pérez Costoya");
MODULE_DESCRIPTION("Manejador con timer");
MODULE_LICENSE("GPL");

static int duracion_ms = 2000; // defecto 2 segundos
module_param(duracion_ms, int, S_IRUSR);
MODULE_PARM_DESC(duracion_ms, "duración en ms. (defecto 2 segundos)");

static struct cdev dispo_desc;
static struct class *dispo_clase;
static int nbytes_escritos = 0;
static int n_escrituras = 0;
static struct timer_list temporizador;
static spinlock_t sincro;

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
	spin_lock_bh(&sincro);
	nbytes_escritos+=count;
	n_escrituras++;
	spin_unlock_bh(&sincro);
        return count;
}
static void fin_plazo(struct timer_list *t) {
	spin_lock_bh(&sincro);
	printk("Fin de plazo %d bytes escritos en %d escrituras\n",
			nbytes_escritos, n_escrituras);
	spin_unlock_bh(&sincro);
	t->expires = jiffies + msecs_to_jiffies(duracion_ms);
	add_timer(t);

}
static struct file_operations dispo_ops = {
        .owner =          THIS_MODULE,
        .open =           abrir,
        .release =        cerrar,
        .write =          escribir,
};
static int __init manejador_init(void) {
        dev_t devnum;
	int minor=0;
	printk("Inicio de manejador de un único dispositivo\n");
	// solicita major para el manejador de un solo dispositivo (minor=0)
        if (alloc_chrdev_region(&devnum, minor, 1, "timer_dispo")<0) {
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
        if ((dispo_clase = class_create(THIS_MODULE, "clase_timer")) == NULL){
                printk(KERN_ALERT "Error in class_create\n");
                unregister_chrdev_region(devnum, 1);
                return -1;
        }
        device_create(dispo_clase, NULL, devnum, NULL, "timer_dispo");
	spin_lock_init(&sincro);
	timer_setup(&temporizador, fin_plazo, 0);
	temporizador.expires = jiffies + msecs_to_jiffies(duracion_ms);
	add_timer(&temporizador);
	return 0;
}
static void __exit manejador_exit(void) {
	del_timer_sync(&temporizador);
	printk("Fin de manejador %d bytes escritos en %d escrituras\n",
			nbytes_escritos, n_escrituras);
        device_destroy(dispo_clase, dispo_desc.dev);
        class_destroy(dispo_clase);
        cdev_del(&dispo_desc);
        unregister_chrdev_region(dispo_desc.dev, 1);

}
module_init(manejador_init);
module_exit(manejador_exit);
