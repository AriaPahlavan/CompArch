#!/bin/bash

filename=${1}
temp="temp.txt"
started=0
printline="no"

if [ -e "$filename" ]
then
  printf ""
else
  echo $filename
	exit 1
fi


while IFS= read -r line; do
  [[ $line == LC-3b-SIM* ]]  && printline="no"
  [[ $line == Bye* ]]        && printline="no"
  [[ $line == Simulat* ]] && printline="no"
  [[ $line == "" ]] && printline="no"
#  [[ $line == "--"* ]] && printline="no"
  [[ $printline == "yes" ]]   && echo "$line" >> $temp
  [[ $line == "" ]] && [[ $started == 1 ]] && printline="yes"
#  [[ $line == "--"* ]] && printline="yes"
  [[ $line == LC-3b-SIM* ]]  && printline="yes" && started=1
  [[ $line == Simulat* ]] && printline="yes"
done < "$filename"

mv ./$temp ./${filename}



exit 0
