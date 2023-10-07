#!/bin/bash
if test "$#" -lt 3
	then echo "Usage: $0 $@"
fi

file=$1
dir=$2
sum=0
cnt=0

shift 2

for arg
	do (( sum += arg ))
		if [ "$arg" -gt 10 ]
			then (( cnt++ ))
		fi
done
echo $sum
echo $cnt 
r=$(echo "$sum" | wc -L)
echo "$r"
if [ -f "$file" ]
	then echo "CNT: $cnt" > "$file"
		 echo "SUM: $sum" >> "$file"
fi

[ -d "$dir" ] && cd $dir | ls -l *.txt | cat *.txt