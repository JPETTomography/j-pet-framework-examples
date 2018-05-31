#!/bin/bash

calib_folder=Run3

if [ -z ${2+x} ]; then
    echo "expected arguments list: type (hld/root/mcGeant)  fileName "
    exit 
fi

rm *.log
./MCGeantAnalysis.x -t $1 -f $2 -p $calib_folder/config.xml  -u $calib_folder/userParams.json -i 2 -l $calib_folder/detectorSetup.json -c $calib_folder/17083123644_TOT_params.root 
