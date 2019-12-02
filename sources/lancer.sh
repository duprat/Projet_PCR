#!/bin/bash

if [ $# != 2 ]
then
    echo "Erreur de lancement"
    echo "$0 [portServeur] [nbClient]"
    echo "/!\ Attention cela lancera  [nbClient] xterm"
    exit
fi

gcc destruction.c -o destruction

make

xterm -T "Serveur" -e "./serveur $1; bash" &
sleep 3s

for i in $(seq 0 1 $3)
do
   xterm -T "Client $i" -e "./client 127.0.1  $1; bash" &
   sleep 2s
done
