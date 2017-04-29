#!/bin/bash

# Regular Colors
blk='\033[0;30m'          # Black
red='\033[0;31m'          # Red
grn='\033[0;32m'          # Green
ylw='\033[0;33m'          # Yellow
blu='\033[0;34m'          # Blue
mgn='\033[0;35m'          # Purple
cyn='\033[0;36m'          # Cyan
nrml='\033[0;37m'         # White
nc='\033[0m'              # No Color

file_num=$1
postfix=".txt"
aoutput="./outputs/actual_output.txt"
eoutput="./outputs/eout${file_num}${postfix}"
numLines=`wc -l < ${eoutput}`
aline="./aline.txt"
eline="./eline.txt"
tempNum=`wc -l < ${aoutput}`

[[ $tempNum -ne $numLines ]] && printf "${red}[ERROR]${nrml} actual file has different number of lines.\n"

touch $eline
touch $aline

for (( i = 0; i < 10; i++ )); do
  sed "${i}q;d" ${aoutput} > ${aline}
  sed "${i}q;d" ${eoutput} > ${eline}
  diff -b -E -w -B ${aline} ${eoutput}
done

rm $eline
rm $aline
