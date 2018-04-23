#!/bin/bash

function executeCommand {
    $@
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
}

executeCommand "mkdir -p build"
executeCommand "cd build"
executeCommand "export CMAKE_LIBRARY_PATH=$CMAKE_LIBRARY_PATH:/framework-dependencies/lib"
executeCommand "export CMAKE_INCLUDE_PATH=$CMAKE_INCLUDE_PATH:/framework-dependencies/include"
executeCommand "source /root-system/bin/thisroot.sh"
executeCommand "cmake .."
executeCommand "cd LargeBarrelAnalysis"
executeCommand "make"
executeCommand "wget http://sphinx.if.uj.edu.pl/~alek/framework_integration_tests/dabc_17025151847.hld"
executeCommand "wget http://sphinx.if.uj.edu.pl/~alek/framework_integration_tests/setupRun3.json"
executeCommand "./LargeBarrelAnalysis.x -t hld -f dabc_17025151847.hld -l setupRun3.json -i 3 -r 0 100"
executeCommand "mkdir outdir"
executeCommand "./LargeBarrelAnalysis.x -t hld -f dabc_17025151847.hld -l setupRun3.json -i 3 -r 0 100 -o outdir"
executeCommand "./LargeBarrelAnalysis.x -t root -f dabc_17025151847.hld.root -l setupRun3.json -i 3 -r 0 100"
executeCommand "sed -i 's/manager.useTask(\"TimeWindowCreator\", \"hld\", \"tslot.raw\");//' ../../LargeBarrelAnalysis/main.cpp"
executeCommand "sed -i 's/manager.useTask(\"TimeCalibLoader\", \"tslot.raw\", \"tslot.calib\");//' ../../LargeBarrelAnalysis/main.cpp"
executeCommand "make"
executeCommand "./LargeBarrelAnalysis.x -t root -f dabc_17025151847.tslot.calib.root   -r 0 100"
executeCommand "cd ../TimeCalibration"
executeCommand "make"
executeCommand "cd ../VelocityCalibration"
executeCommand "make"
executeCommand "cd ../NewAnalysisTemplate"
executeCommand "make"
