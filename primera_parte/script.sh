#!/bin/bash

test $(id -u) = 0 || { echo "Ejecuta $0 con sudo" >&2; exit 1; }
make &> /dev/null

# crea todos los grupos
mkdir /sys/fs/cgroup/profesores
mkdir /sys/fs/cgroup/estudiantes
mkdir /sys/fs/cgroup/estudiantes/grado
mkdir /sys/fs/cgroup/estudiantes/master

# especifica los pesos de cada grupo del nivel superior
echo 50 > /sys/fs/cgroup/profesores/cpu.weight  # 1/3
echo 100 > /sys/fs/cgroup/estudiantes/cpu.weight  # 2/3

# habilita el reparto de procesador en el nivel inferior
echo "+cpu" > /sys/fs/cgroup/cgroup.subtree_control
echo "+cpu" > /sys/fs/cgroup/estudiantes/cgroup.subtree_control

# especifica los pesos de cada grupo del nivel inferior
echo 16 > /sys/fs/cgroup/estudiantes/grado/cpu.weight  # 1/6
echo 50 > /sys/fs/cgroup/estudiantes/master/cpu.weight  # 1/2

# tres procesos de profesores: sustituya ... por grupo correspondiente
taskset 1 ./programa 100000000 9 3 0 0 0 0 2>/sys/fs/cgroup/profesores/cgroup.procs &

# tres procesos de alumnos de grado: sustituya ... por grupo correspondiente
taskset 1 ./programa 100000000 9 3 3 0 0 0 2>/sys/fs/cgroup/estudiantes/grado/cgroup.procs &

# tres procesos de alumnos de máster: sustituya ... por grupo correspondiente
taskset 1 ./programa 100000000 9 3 6 0 0 0 2>/sys/fs/cgroup/estudiantes/master/cgroup.procs &

wait # espera que finalicen las tres aplicaciones

# destruye los grupos
rmdir /sys/fs/cgroup/profesores
rmdir /sys/fs/cgroup/estudiantes/grado
rmdir /sys/fs/cgroup/estudiantes/master
rmdir /sys/fs/cgroup/estudiantes
