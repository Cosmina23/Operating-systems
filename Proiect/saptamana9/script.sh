#!/bin/bash

if [ "$#" -ne 1 ]
then
    echo "Usage $0 <character>"
    exit 1
fi
c=0
while read linie
do
    r=$( echo "$linie" | grep -E '^[A-Z]{1,1}[a-zA-Z0-9\ \,\?\!\.]*(\.|\?|\!)$' | grep -vE '(\,[\ ]*si)' | grep -E "$1+")
    if [[ -n "$r" ]]
    then
	(( c++ )) 
    fi    
done
echo "$c"
