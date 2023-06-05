#!/bin/sh
test $# -eq 1 || { echo "uso: $0 dir.virtual.kernel(hexa)" 2>&1; exit 1; }
test -f modulos/virt_to_phys_kernel_mod/virt_to_phys_kernel.ko || { echo "debe entrar al directorio modulos/virt_to_phys_kernel_mod y compilar ejecutando make" 2>&1; exit 1; }
test $(id -u) -eq 0 || { echo "debe ejecutar este mandato como superusuario" 2>&1; exit 1; }
dmesg --clear
insmod modulos/virt_to_phys_kernel_mod/virt_to_phys_kernel.ko dir_param=$1
rmmod virt_to_phys_kernel
echo "$1: " $(dmesg | grep $1 | awk '{print $NF}')
