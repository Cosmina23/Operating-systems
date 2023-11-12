#!/bin/bash

if [ "$#" -lt 2 ]
then
    echo "Prea putine argumente"
fi

file="$2"
dir="$1"
cnt=0
sum=0

echo "" > "$2"

for file in $1/*.txt
do
    cnt=$(wc -c < "$file")
    ((sum+=cnt))
    echo "$file $cnt" >> "$2"
done

echo "TOTAl $sum" >> "$2"
