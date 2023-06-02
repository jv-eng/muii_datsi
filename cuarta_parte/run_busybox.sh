#!/bin/bash
set -x
docker run -it --rm -u 10000:10000 -v $PWD/html:/data busybox
