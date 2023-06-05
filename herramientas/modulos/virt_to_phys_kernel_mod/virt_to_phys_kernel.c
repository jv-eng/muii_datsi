// Imprime la dirección física asociada a una dirección virtual del kernel.
// Solo es válida para direcciones mapeadas directamente.
// parámetro ("dir_param": dirección virtual en hexadecimal). El parámetro
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/mm_types.h>

MODULE_LICENSE("GPL");

static char * dir_param=0;
module_param(dir_param, charp, S_IRUGO);
MODULE_PARM_DESC(dir_param, "dir virtual del kernel de la que se quiere conocer su dirección física (en hexadecimal)");

static int __init virt_to_phys_kernel_init(void) {
        unsigned long dirv;
        if (dir_param) {
                if (kstrtoul(dir_param, 16, &dirv)) return 0;
	        printk("%lx %llx\n", dirv, virt_to_phys((void *)dirv));
	}
	return 0;
}


static void __exit virt_to_phys_kernel_exit(void) {
}

module_init(virt_to_phys_kernel_init);
module_exit(virt_to_phys_kernel_exit);

