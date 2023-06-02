#!/bin/bash

make &> /dev/null
# creo la cuenta del concursante
sudo useradd -m -s /bin/bash concursante &> /dev/null
sudo passwd -d concursante &> /dev/null

# se queda a la espera de filtraciones
su concursante << EOF
chmod 777 /home/concursante
rm  -f /tmp/canal
touch /tmp/canal
chmod 666 /tmp/canal
rm -f /home/concursante/canal
touch /home/concursante/canal
chmod 666 /home/concursante/canal
nc -l -p 6666 & 
tail -F /tmp/canal &
tail -F /home/concursante/canal &
./lee_cola_IPC &
echo "A la escucha de filtraciones; pulse return para terminar: "
read v < /dev/tty
jobs -p | xargs kill -9 2>/dev/null
EOF

# destruyo la cuenta del concursante
sudo userdel -r concursante &> /dev/null
