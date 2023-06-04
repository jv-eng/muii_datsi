#!/bin/bash
#a modificar, realiza la ejecucion del codigo entregado
make &> /dev/null

unshare -r -n -i -m bash << EOF
    mkdir fake_dir
    mount --bind fake_dir /tmp #montamos el directorio falso
    mount -o bind,ro /home /home #el directorio home solo de lectura

    #creamos ficheros
    echo HoLa > /tmp/fichero
    echo hOlA > /tmp/solucion

    #ejecutamos
    ./toggleCaps-trojan /tmp/fichero
    cmp /tmp/fichero /tmp/solucion && echo "reto superado" || echo "reto no superado"

    #liberamos recursos
    umount /home
    umount /tmp
    rm -r fake_dir
EOF
