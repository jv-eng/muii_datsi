#!/bin/bash
set -x
docker run -d -it --name web_server3  -p8081:80 -v $PWD/html:/usr/share/nginx/html -v /etc/shadow:/hostetc nginx
