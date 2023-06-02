#!/bin/bash
test $(id -u) = 0 || { echo "Ejecuta $0 con sudo" >&2; exit 1; }
make &> /dev/null

# crea todos los grupos

# especifica los pesos de cada grupo del nivel superior

# habilita el reparto de procesador en el nivel inferior

# especifica los pesos de cada grupo del nivel inferior

# tres procesos de profesores: sustituya ... por grupo correspondiente
taskset 1 ./programa 100000000 9 3 0 0 0 0 2>/sys/fs/cgroup/... &

# tres procesos de alumnos de grado: sustituya ... por grupo correspondiente
taskset 1 ./programa 100000000 9 3 3 0 0 0 2>/sys/fs/cgroup/... &

# tres procesos de alumnos de máster: sustituya ... por grupo correspondiente
taskset 1 ./programa 100000000 9 3 6 0 0 0 2>/sys/fs/cgroup/... &

wait # espera que finalicen las tres aplicaciones

# destruye los grupos

