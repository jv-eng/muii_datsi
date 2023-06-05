// Imprime el valor que contiene la dirección virtual del kernel recibida como
// parámetro ("dir_param": dirección virtual del kernel en hexadecimal).
// El parámetro "tam" (por defecto, 1) indica cuántos bytes se leen
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/sched/task.h>
#include <linux/mm_types.h>

MODULE_LICENSE("GPL");

static char * dir_param=0;
static int tam=1;
module_param(dir_param, charp, S_IRUGO);
module_param(tam, int, S_IRUGO);
MODULE_PARM_DESC(dir_param, "dir virtual del kernel (en hexadecimal)");
MODULE_PARM_DESC(tam, "nº de bytes que se pretenden leer (por defecto, 1)");


static int __init read_virt_kernel_init(void) {
        unsigned long dirv;
	int i;
        if (dir_param) {
                if (kstrtoul(dir_param, 16, &dirv)) return 0;
                if (dirv) {
                        for (i=0; i<tam; i++)
                                printk("%lx %x\n", dirv, *(((unsigned char *)dirv)+i));

		}
	}
	return 0;
}


static void __exit read_virt_kernel_exit(void) {
}

module_init(read_virt_kernel_init);
module_exit(read_virt_kernel_exit);

