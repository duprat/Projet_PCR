#!/bin/bash

if [ $# != 3 ]
then
    echo "Erreur de lancement"
    echo "$0 [id_shm1] [id_shm2] [id_sem]"
    exit
fi
ipcrm -m $1
ipcrm -m $2
ipcrm -s $3

