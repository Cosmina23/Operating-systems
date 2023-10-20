#!/bin/bash

while read linie
do
    r=$( echo "$linie" | grep -E '^[A-Z]{1,1}[a-zA-Z0-9 ,]*\.$' | grep -vE '(si[ ]*,)' | grep -v 'n[pb]')
    if [[ -n "$r" ]]
    then
	echo "P"
    else
	echo "No P"
    fi
    
done

