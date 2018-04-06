#!/bin/bash

calib_folder=calib_run_3

if [ -z ${1+x} ]; then echo "give a file name as argument"; exit; fi
rm *.log
./MCGeantAnalysis.x -t root -o data_output -r 1 5000  -f $1 -p $calib_folder/conf_trb3.xml  -u $calib_folder/userParams.json -i 2 -l $calib_folder/detectorSetupRun2345.json -c $calib_folder/tot_calib_dabc17083123644_a.root 
