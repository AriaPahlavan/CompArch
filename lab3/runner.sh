#! /bin/bash

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

num_tests=$1
exe="simulate"
control_store="ucode3"
asm="./tests/test"
cmd="./inputs/in"
output="./outputs/aout"
postfix=".txt"
cleaner_script="cleaner.sh"
tester_script="tester.sh"
exitCode=0
num_errors=0

printf "${blu}[INFO]${nrml} Running ${num_tests} tests... \n"

for (( j = 0; j < $num_tests; j++ )); do
  i=$((j+1))
  # echo "Test ${i}:"
  ./${exe} ${control_store} ${asm}${i}  < ${cmd}${i}${postfix} > ${output}${i}${postfix}
  temp=$?
  if [ $temp -ne 0 ]; then
    exitCode=$temp
    printf "${red}[ERROR]${nrml} Invalid file name in simulator. (${i})\n"
  fi
  ./${cleaner_script} ${output}${i}${postfix}
  temp=$?
  if [ $temp -ne 0 ]; then
    exitCode=$temp
    printf "${red}[ERROR]${nrml} Invalid file name in cleaner. (${i})\n"
  fi
  ./${tester_script} ${i} && printf "${grn}[PASS]${nrml} Expected value matched! (${i})\n"
  temp=$?
  if [ $temp -ne 0 ]; then
    exitCode=$temp
    printf "${red}[FAIL]${nrml} Unmached values found. (${i})\n"
    ((num_errors++))
  fi


done

# [[ $exitCode == 0 ]] &&
# || printf "${red}[FAIL]${nrml} Test failure occured.\n"
printf "${blu}[INFO]${nrml} $((num_tests-num_errors))/$num_tests passed.\n"

exit $exitCode
