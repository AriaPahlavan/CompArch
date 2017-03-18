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
  [[ $line == Simulating* ]] && printline="no"
  [[ $line == "" ]] && printline="no"
  [[ $line == "--"* ]] && printline="no"
  [[ $line == "STATE_NUMBER"* ]] && printline="no"
  [[ $line == "BUS"* ]] && printline="no"
  [[ $line == "IR"* ]] && printline="no"
  [[ $line == "MDR"* ]] && printline="no"
  [[ $printline == "yes" ]]   && echo "$line" >> $temp
  [[ $line == "" ]] && [[ $started == 1 ]] && printline="yes"
  [[ $line == "--"* ]] && printline="yes"
  [[ $line == "STATE_NUMBER"* ]] && printline="yes"
  [[ $line == "BUS"* ]] && printline="yes"
  [[ $line == "IR"* ]] && printline="yes"
  [[ $line == "MDR"* ]] && printline="yes"
  [[ $line == LC-3b-SIM* ]]  && printline="yes" && started=1
  [[ $line == Simulating* ]] && printline="yes"
done < "$filename"

mv ./$temp ./${filename}

exit 0
