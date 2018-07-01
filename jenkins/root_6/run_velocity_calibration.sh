#!/bin/bash
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16315170022.hld.root -p conf_trb3.xml -u userParams_root6.json -i 2 -o results_root_6&
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16315215024.hld.root -p conf_trb3.xml -u userParams_root6.json -i 2 -o results_root_6&
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16316203534.hld.root -p conf_trb3.xml -u userParams_root6.json -i 2 -o results_root_6&
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16317101413.hld.root -p conf_trb3.xml -u userParams_root6.json -i 2 -o results_root_6&
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16317230553.hld.root -p conf_trb3.xml -u userParams_root6.json -i 2 -o results_root_6&

wait
../../build/VelocityCalibration/estimateVelocity results_root6.txt calibrationRoot6