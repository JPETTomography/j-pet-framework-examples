#!/bin/bash

if [ -z ${3+x} ]; then
    echo "expected arguments: input_file_name setup_description_json run_number "
    exit 
fi

rm *.log
./MCGeantAnalysis.x -t mcGeant -f $1 -u userParams.json -l $2 -i $3
