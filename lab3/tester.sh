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
aoutput="./outputs/aout${file_num}${postfix}"
eoutput="./outputs/eout${file_num}${postfix}"

# printf "${blu}[INFO]${nrml} Comparing aout${file_num}${postfix} with eout${file_num}${postfix}\n"

diff ${aoutput} ${eoutput}
