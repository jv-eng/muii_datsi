#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_AUTHOR("Fernando Pérez Costoya");
MODULE_DESCRIPTION("Módulo mínimo");
MODULE_LICENSE("GPL");

MODULE_PARM_DESC(entero, "UN ENTERO");
static int entero = 666;
// parámetro accesible en sysfs: root puede leerlo y modificarlo
module_param(entero, int, S_IRUSR|S_IWUSR);

static int __init mod1_init(void) {
	printk("Inicio de mod1: parámetro %d\n", entero);
	return 0;
}
static void __exit mod1_exit(void) {
	printk("Fin de mod1: parámetros %d\n", entero);
}
module_init(mod1_init); // función de entrada del módulo
module_exit(mod1_exit); // función de salida del módulo
