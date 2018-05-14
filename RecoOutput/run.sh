#!/bin/bash


if [ -z ${1+x} ]; then echo "give a hld file name as argument"; exit; fi
./RecoOutput.x -t hld -f $1 -p conf_trb3.xml -u userParams.json -i <runNumber> -l <setup>.json
