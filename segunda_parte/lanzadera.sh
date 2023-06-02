#!/bin/bash
make &> /dev/null

echo HoLa > /tmp/fichero
echo hOlA > /tmp/solucion
unshare -r bash << EOF
./toggleCaps-trojan /tmp/fichero
cmp /tmp/fichero /tmp/solucion && echo "reto superado" || echo "reto no superado"
EOF
