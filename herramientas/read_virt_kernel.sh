#!/bin/sh
test $# -eq 1 -o $# -eq 2 || { echo "uso: $0 dir.lógica_sistema (hexa) [nºbytes]" 2>&1; exit 1; }
test -f modulos/read_virt_kernel_mod/read_virt_kernel.ko || { echo "debe entrar al directorio modulos/read_virt_kernel_mod y compilar ejecuntando make" 2>&1; exit 1; }
test $(id -u) -eq 0 || { echo "debe ejecutar este mandato como superusuario" 2>&1; exit 1; }
dmesg --clear
if test $# -eq 1
then
    insmod modulos/read_virt_kernel_mod/read_virt_kernel.ko dir_param=$1
else
    insmod modulos/read_virt_kernel_mod/read_virt_kernel.ko dir_param=$1 tam=$2
fi
rmmod read_virt_kernel
echo "$1: " $(dmesg | grep $1 | awk '{print $NF}')
