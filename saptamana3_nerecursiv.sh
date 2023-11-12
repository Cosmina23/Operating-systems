#!/bin/bash

if [ "$#" -lt 2 ]
then
    echo "Prea putine argumente"
fi

for file in $1/*.txt
do
    chmod +"$2" "$file"
done

	   
