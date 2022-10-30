// USA BITFIELDS; CUIDADO: no portables; dependen del compilador y procesador
#include <sys/io.h>
#include <stdio.h>
#include <stdint.h>
#define RTC_REG 0x70   // registro de selección de registro
#define RTC_DAT 0x71   // registro lectura/escritura de registro seleccionado
#define RTC_SEG 0x00   // registro que mantiene segundos
#define RTC_MIN 0x02   // registro que mantiene minutos
#define RTC_HORA 0x04  // registro que mantiene mes
#define RTC_DIA 0x07   // registro que mantiene hora
#define RTC_MES 0x08   // registro que mantiene mes
#define RTC_ANNO 0x09  // registro que mantiene año
#define RTC_STATB 0x0B // reg. est. B: bit 1: 24|12 horas; bit 2: binario|BCD

typedef union reg {
	struct { // formato reg. estado B
		uint8_t no_usado:1;
		uint8_t formato24h:1;
		uint8_t binario:1;
		uint8_t resto:5;
	};
	struct { // formato reg. hora
		uint8_t hora:7;
		uint8_t pm:1;
	};
	struct { // formato BCD
		uint8_t BCD_nibble1:4;
		uint8_t BCD_nibble2:4;
	};
	uint8_t valor; // formato general
} reg_t;

struct fecha {int seg, min, hora, dia, mes, anno;} fecha;

reg_t leer_reg(uint8_t reg){
	reg_t r;
	outb(reg, RTC_REG);
	r.valor=inb(RTC_DAT);
	return r; }

int a_binario(int binario, reg_t r) { // de BCD a binario si necesario
	return binario ? r.valor : r.BCD_nibble2 * 10 + r.BCD_nibble1; }


int main() {
	if (ioperm(RTC_REG, 2, 1) < 0) { perror("ioperm"); return 1; }

	reg_t regB = leer_reg(RTC_STATB); // accede a reg. estado B

	fecha.seg  = a_binario(regB.binario, leer_reg(RTC_SEG)); 
	fecha.min  = a_binario(regB.binario, leer_reg(RTC_MIN)); 
	fecha.dia  = a_binario(regB.binario, leer_reg(RTC_DIA)); 
	fecha.mes  = a_binario(regB.binario, leer_reg(RTC_MES)); 
	fecha.anno = a_binario(regB.binario, leer_reg(RTC_ANNO)); 

	reg_t r = leer_reg(RTC_HORA); 
	int pm=0;
	if (!regB.formato24h && r.pm){ // hora PM si formato 12h y bit de mayor peso activo
		pm=1;  r.pm=0;; // elimino bit de mayor peso
	}
	fecha.hora = a_binario(regB.binario, r); 
	if (pm) fecha.hora = (fecha.hora + 12) % 24; // de PM a 24h
						 
	printf("%02d:%02d:%02d %02d/%02d/%02d\n", fecha.hora,fecha.min, fecha.seg, fecha.dia, fecha.mes, fecha.anno);
	return 0;
} 
