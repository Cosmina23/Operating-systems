#!/bin/bash

#-L/-h legatura simbolica
#grep -E '^[A-Z]{1,1}[a-zA-Z0-9 ,]*\.$' | grep -vE '(si[ ]*,)' | grep -v 'n[pb]'
#scriere cale fisier daca se gasesste regex in fisier
#count legatura simbolica
#parcurgere director

if [ "$#" -lt 2 ]
then
    echo "Prea putine argumente"
fi

count=0

for entry in $1/*
do

    if [ -f "$entry" ]
    then
	r=$(cat "$entry" | grep -E '^[A-Z]{1,1}[a-zA-Z0-9 ,]*\.$' | grep -vE '(si[ ]*,)' | grep -v 'n[pb]');
	if [[ -n $r ]]
	then
	    echo "$entry" >> "$2"
	fi
    fi
    if [ -L "$entry" ]
    then
	(( count++ ))
    fi
    if [ -d "$entry" ]
    then
	count2=$(bash $0 $entry $2)
	count=$((count + count2))
    fi
done
    

echo "$count"	      
