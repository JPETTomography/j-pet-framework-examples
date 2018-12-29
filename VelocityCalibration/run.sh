./VelocityCalibration.x -t zip -f ./firstFile.xz -p conf_trb3.xml -u userParams.json -i 2 -c ../../CalibrationFiles/?_RUN/<TOTconfigFile> -l ../../CalibrationFiles/?_RUN/detectorSetupRun?.json & 
./VelocityCalibration.x -t zip -f ./secondFile.xz -p conf_trb3.xml -u userParams.json -i 2 -c ../../CalibrationFiles/?_RUN/<TOTconfigFile> -l ../../CalibrationFiles/?_RUN/detectorSetupRun?.json & 
./VelocityCalibration.x -t zip -f ./thirdFile.xz -p conf_trb3.xml -u userParams.json -i 2 -c ../../CalibrationFiles/?_RUN/<TOTconfigFile> -l ../../CalibrationFiles/?_RUN/detectorSetupRun?.json & 
./VelocityCalibration.x -t zip -f ./fourthFile.xz -p conf_trb3.xml -u userParams.json -i 2 -c ../../CalibrationFiles/?_RUN/<TOTconfigFile> -l ../../CalibrationFiles/?_RUN/detectorSetupRun?.json & 
./VelocityCalibration.x -t zip -f ./fifthFile.xz -p conf_trb3.xml -u userParams.json -i 2 -c ../../CalibrationFiles/?_RUN/<TOTconfigFile> -l ../../CalibrationFiles/?_RUN/detectorSetupRun?.json & 

wait

./estimateVelocity <resultFile>
