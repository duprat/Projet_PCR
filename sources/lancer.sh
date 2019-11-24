#!/bin/bash

if [ $# != 3 ]
then
    echo "Erreur de lancement"
    echo "$0 [portServeur] [ipServeur] [nbClient]"
    echo "/!\ Attention cela lancera  [nbClient] xterm"
    exit
fi

gcc destruction.c -o destruction
make

xterm -T "Serveur" -e "./serveur --port $1; bash" &
sleep 3s

for i in $(seq 1 $3)
do
   xterm -T "Client $i" -e "./client --ip $2 --port $1; bash" &
   sleep 2s
done
