#!/bin/bash

test $(id -u) = 0 || { echo "Ejecuta $0 con sudo" >&2; exit 1; }
make &> /dev/null

# crea los dos grupos
mkdir /sys/fs/cgroup/usuario1
mkdir /sys/fs/cgroup/usuario2

# hace disponible el controlador "cpu" a los nodos hijos 
echo "+cpu" > /sys/fs/cgroup/cgroup.subtree_control

# especifica los pesos de cada grupo
echo 75 > /sys/fs/cgroup/usuario1/cpu.weight  # 1/3
echo 150 > /sys/fs/cgroup/usuario2/cpu.weight # 2/3

echo "dos usuarios con pesos 1/3 y 2/3 que ejecutan 3 procesos de prioridad 0"
taskset 1 ./programa 100000000 6 3 0 0 0 0 2>/sys/fs/cgroup/usuario1/cgroup.procs &
taskset 1 ./programa 100000000 6 3 3 0 0 0 2>/sys/fs/cgroup/usuario2/cgroup.procs &
wait

# destruye los dos grupos
rmdir /sys/fs/cgroup/usuario1
rmdir /sys/fs/cgroup/usuario2 
