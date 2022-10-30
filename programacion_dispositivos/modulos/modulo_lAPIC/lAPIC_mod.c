#include <linux/init.h>
#include <linux/module.h>
#include <asm/io.h>
MODULE_LICENSE("GPL");

#define LOCAL_APIC_BASE_ADDRESS 0xfee00000
struct lapic_info {
	uint32_t reservado[8];
	uint32_t ID;
	uint32_t relleno[3];
	uint32_t version; // nº versión: 8 bits de menor peso
	// ... no usados por el programa
};
static int lAPIC_init(void) {
	int tam=4096;
	struct lapic_info volatile *lapic; // volatile elimina optimizaciones de compilador

	// obtiene rango dir. lógicas sistema asociadas a dir. físicas dispositivo
	if ((lapic = ioremap(LOCAL_APIC_BASE_ADDRESS, tam))==NULL) {
                printk("Error en ioremap\n");
                return -1;
        }
	printk("Acceso dir física %x usando %p\n", LOCAL_APIC_BASE_ADDRESS, lapic);
	printk("Local APIC ID: %d Versión %d\n", lapic->ID, lapic->version&0xFF);
	iounmap(lapic);
	return 0;
} 
static void lAPIC_exit(void) {}
module_init(lAPIC_init); 
module_exit(lAPIC_exit); 
