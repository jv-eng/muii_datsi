#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/i8253.h>
#include <asm/io.h>

//declaracion de funciones
/*void set_spkr_frequency(unsigned int frequency);
void spkr_on(void);
void spkr_off(void);*/


///////////////////////////////////////////////////////
/*implementacion*/
//////////////////////////////////////////////////////

//fijar frecuencia
void set_spkr_frequency(unsigned int frequency) {
	//variables locales
	uint32_t port1 = 0x42;
	uint32_t port2 = 0x43;
	int freq = PIT_TICK_RATE / frequency;
	unsigned long flags = 0;

	printk("spkr set frequency: %d\n", frequency);


	//inicio seccion critica
	raw_spin_lock_irqsave(&i8253_lock, flags);
	
	//escritura en el dispositivo
	outb_p(0xB6, port2);
	outb_p(freq & 0xff, port1);
	outb((freq >> 8) & 0xff, port1);

	//fin seccion critica
	raw_spin_unlock_irqrestore(&i8253_lock, flags);
}

//encender altavoz
void spkr_on(void) {
	//variables locales
	uint32_t port = 0x61;
	unsigned long flags = 0;

	//inicio seccion critica
	raw_spin_lock_irqsave(&i8253_lock, flags);
	
	//activar mute
	outb(inb(port) | 3, port);

	//fin seccion critica
	raw_spin_unlock_irqrestore(&i8253_lock, flags);

	printk("spkr ON\n");

}

//apagar altavoz
void spkr_off(void) {
	//variables locales
	uint32_t port = 0x61;
	unsigned long flags = 0;

	//inicio seccion critica
	raw_spin_lock_irqsave(&i8253_lock, flags);
	
	//desactivar mute
	outb(inb_p(port) & 0xFC, port);

	//fin seccion critica
	raw_spin_unlock_irqrestore(&i8253_lock, flags);

	printk("spkr OFF\n");
}
