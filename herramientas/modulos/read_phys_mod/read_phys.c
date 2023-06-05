// Imprime el valor que contiene la dirección física recibida como
// parámetro ("dir_param": dirección física en hexadecimal). El parámetro
// "tam" (por defecto, 1) indica cuántos bytes se leen
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/memremap.h>
#include <linux/sched.h>
#include <linux/mm_types.h>

MODULE_LICENSE("GPL");

static char * dir_param=0;
static int tam=1;
module_param(dir_param, charp, S_IRUGO);
module_param(tam, int, S_IRUGO);
MODULE_PARM_DESC(dir_param, "dir física que se pretende leer (en hexadecimal)");
MODULE_PARM_DESC(tam, "nº de bytes que se pretenden leer (por defecto, 1)");

static int __init readp_init(void) {
	unsigned char *dirv;
	unsigned long dirf, frame, offset;
	int i;
	if (dir_param) {
		if (kstrtoul(dir_param, 16, &dirf)) return 0;
		frame  = dirf &  PAGE_MASK;
		offset = dirf & ~PAGE_MASK;
		dirv = memremap(frame, PAGE_SIZE, MEMREMAP_WB);
		if (dirv) {
			for (i=0; i<tam; i++) 
				printk(KERN_ALERT "%lx %x\n", dirf, *(dirv+i+offset));
			memunmap((void *)frame);
		}
	}
	return 0;
}


static void __exit readp_exit(void) {
}

module_init(readp_init);
module_exit(readp_exit);

