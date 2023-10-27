#!/bin/bash
#-L/-h - legatura simbolica

#grep -E '^[A-Z]{1,1}[a-zA-Z0-9 ,]*\.$' | grep -vE '(si[ ]*,)' | grep -v 'n[pb]'

if [ "$#" -lt 2 ]
then
    echo "Prea putine argumente"
fi

count=0

for entry in $1/*
do
    if [ -f "$entry" ]
    then
	r=$(cat "$entry" | grep -E '^[A-Z]{1,1}[a-zA-Z0-9 ,]*\.$' | grep -vE '(si[ ]*,)' | grep -v 'n[pb]')
	if [[ -n $r ]]
	then
	    echo "$entry" >> "$2"
	fi
    
    elif [ -L "$entry" ]
    then
	count='expr $count + 1'
    
    elif [ -d "$entry" ]
    then
	count2=$(bash $0 $file $2)
	(count='expr $count + $count2')
    fi
echo "$count"

done

