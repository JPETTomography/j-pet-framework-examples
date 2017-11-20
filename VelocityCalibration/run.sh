./VelocityCalibration.x -t zip -f ./firstFile.xz -p conf_trb3.xml -u userParams.json -i 2 -c 20160712_params.root -b
./VelocityCalibration.x -t zip -f ./secondFile.xz -p conf_trb3.xml -u userParams.json -i 2 -c 20160712_params.root -b

./estimateVelocity
