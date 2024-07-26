#!/bin/bash

if [ -z ${1+x} ]; then echo "Give a direcory name with .cat.evt.root files as argument"; exit; fi

for file in ${1}/*cat.evt.root ; do
./ListmodeExport.x -t root -f ${file} -u userParams.json -o ${1};
done
