#!/bin/bash

#Să se scrie un shell/bash script care primește ca parametru un director și un caracter
#Script-ul va scana acel director (recursiv/nerecursiv) și pentru toate fisierele cu extensia .txt va seta dreptul după cum primește ca parametru. 

if [ "$#" -lt 2 ]
then
    echo "Prea putine argumente"
fi


for file in $1/*.txt
do
    chmod +"$2" "$file"
done


for file in $1/*
do
    if [ -d "$file" ]
    then
	bash $0 $file $2
    fi
done
