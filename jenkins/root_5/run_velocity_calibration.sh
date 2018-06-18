#!/bin/bash
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16315170022.hld -p conf_trb3.xml -u userParams.json -i 2 -o results_root_5&
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16315215024.hld -p conf_trb3.xml -u userParams.json -i 2 -o results_root_5&
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16316203534.hld -p conf_trb3.xml -u userParams.json -i 2 -o results_root_5&
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16317101413.hld -p conf_trb3.xml -u userParams.json -i 2 -o results_root_5&
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16317230553.hld -p conf_trb3.xml -u userParams.json -i 2 -o results_root_5&

wait
../../build/VelocityCalibration/estimateVelocity