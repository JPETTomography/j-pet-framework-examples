#!/bin/bash
INPUT_FILE=$1
INPUT_FILE_TEST=$2

if [ -z $1 ] ; then
    echo "No input file!"
    exit 1
fi

if [ -z $2 ] ; then
    echo "No test input file!"
    exit 2
fi

input=( $(<$INPUT_FILE) )
inputlen=${#input[*]}

input_test=( $(<$INPUT_FILE_TEST) )
inputlen_test=${#input_test[*]}

if [ $inputlen -ne $inputlen_test ] ; then
    echo "Input lenght not equals test input lenght!"
    exit 3
fi

x=0

while [ $x + 2 -lt $inputlen ]; do
    result_num = $x * 3 + 1
    diff_num = $x * 3 + 2
    
    diff_val = $($input[$result_num] - $input_test[$result_num])
    diff_val = ${diff_val#-}
    
    if [ $diff_val -gt $input[$diff_num] ] ; then
        echo "Difference is greather then input file accuracy"
        exit 4
    fi
    
    if [ diff_val -gt $input_test[$diff_num] ] ; then
        echo "Difference is greather then test file accuracy"
        exit 5
    fi
    
    $x = $x + 1
done

exit 0
