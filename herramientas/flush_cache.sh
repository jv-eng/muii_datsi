#!/bin/sh
test $(id -u) -eq 0 || { echo "debe ejecutar este mandato como superusuario" 2>&1; exit 1; }
sync
echo 1 >  /proc/sys/vm/drop_caches
