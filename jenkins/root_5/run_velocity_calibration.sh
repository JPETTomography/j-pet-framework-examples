#!/bin/bash
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16315170022.hld.root -p conf_trb3.xml -u userParams_root5.json -i 2 -o results_root_5&
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16315215024.hld.root -p conf_trb3.xml -u userParams_root5.json -i 2 -o results_root_5&
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16316203534.hld.root -p conf_trb3.xml -u userParams_root5.json -i 2 -o results_root_5&
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16317101413.hld.root -p conf_trb3.xml -u userParams_root5.json -i 2 -o results_root_5&
../../build/VelocityCalibration/VelocityCalibration.x -t root -f ../dataForVelocity/dabc_16317230553.hld.root -p conf_trb3.xml -u userParams_root5.json -i 2 -o results_root_5&

wait
../../build/VelocityCalibration/estimateVelocity results_root5.txt calibrationRoot5