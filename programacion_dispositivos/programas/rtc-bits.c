#include <sys/io.h>
#include <stdio.h>
#include <stdint.h>
#include "bits.h"
#define RTC_REG 0x70   // registro de selección de registro
#define RTC_DAT 0x71   // registro lectura/escritura de registro seleccionado
#define RTC_SEG 0x00   // registro que mantiene segundos
#define RTC_MIN 0x02   // registro que mantiene minutos
#define RTC_HORA 0x04  // registro que mantiene mes
#define RTC_DIA 0x07   // registro que mantiene hora
#define RTC_MES 0x08   // registro que mantiene mes
#define RTC_ANNO 0x09  // registro que mantiene año
#define RTC_STATB 0x0B // reg. est. B: bit 1: 24|12 horas; bit 2: binario|BCD
struct fecha {int seg, min, hora, dia, mes, anno;} fecha;
uint8_t leer_reg(uint8_t reg){
	outb(reg, RTC_REG); return inb(RTC_DAT); }
int a_binario(int binario, uint8_t n) { // de BCD a binario si necesario
	return (binario ? n : (n>>4) * 10) + (n & 0xF); }
int main() {
	if (ioperm(RTC_REG, 2, 1) < 0) { perror("ioperm"); return 1; }
        uint8_t formato = leer_reg(RTC_STATB); // accede a reg. estado B
	int binario = bit_es_igual_uno(formato, 2); // binario o BCD
	int formato24h = bit_es_igual_uno(formato, 1); // 24h o 12h
	fecha.seg  = a_binario(binario, leer_reg(RTC_SEG)); 
	fecha.min  = a_binario(binario, leer_reg(RTC_MIN)); 
	fecha.dia  = a_binario(binario, leer_reg(RTC_DIA)); 
	fecha.mes  = a_binario(binario, leer_reg(RTC_MES)); 
	fecha.anno = a_binario(binario, leer_reg(RTC_ANNO)); 
	uint8_t leido = leer_reg(RTC_HORA); 
	int pm=0;
	if (!formato24h && (bit_es_igual_uno(leido, 7))){ // hora PM si formato 12h y bit de mayor peso activo
		pm=1; leido = bit_fija_a_cero(leido, 7); // elimino bit de mayor peso
	}
	fecha.hora = a_binario(binario, leido); 
	if (pm) fecha.hora = (fecha.hora + 12) % 24; // de PM a 24h
						 
	printf("%02d:%02d:%02d %02d/%02d/%02d\n", fecha.hora,fecha.min, fecha.seg, fecha.dia, fecha.mes, fecha.anno);
	return 0;
} 
