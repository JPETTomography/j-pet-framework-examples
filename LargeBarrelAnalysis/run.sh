#!/bin/bash


if [ -z ${1+x} ]; then echo "give a hld file name as argument"; exit; fi
./LargeBarrelAnalysisExtended.x -t hld -f $1 -p conf_trb3.xml -u userParams.json -i 43 -l large_barrel.json
