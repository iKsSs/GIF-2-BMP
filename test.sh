#!/bin/bash

if [ $# -eq 0 ]
then
	declare -a arr=($(ls test | grep .gif | grep -o "^[^.]*"))
else
	declare -a arr=( $1 ); 
fi

mkdir -p err log out 

for pom in "${arr[@]}"
do 
	./gif2bmp -i ./test/${pom}.gif -o ./out/${pom}.bmp >./log/${pom}.log 2>./err/${pom}.err
done