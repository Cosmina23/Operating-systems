#!/bin/bash

#regex care verifica daca o data de forma dd-MM-yyyy hh:mm:ss e valida
while read line
do
    r=$(echo "$line" | grep -E '^((0[1-9])|(1[0-9])|(2[0-9])|(3[0-1]))-((0[1-9])|(1[0-2]))-[0-9]{4}\ ([0-1][0-9]|2[0-4]):([0-5][0-9]):([0-5][0-9])' | grep -vE '^(31|30)-(02)-[0-9]{4}\ ([0-1][0-9]|2[0-4]):([0-5][0-9]):([0-5][0-9])' )
    echo "$r"
done 
