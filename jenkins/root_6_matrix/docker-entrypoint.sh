#!/bin/bash

function executeCommand {
    $@
    rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
    echo "Exit code[" $@ "]: $rc"
}


executeCommand "export CMAKE_LIBRARY_PATH=$CMAKE_LIBRARY_PATH:/framework-dependencies/lib"
executeCommand "export CMAKE_INCLUDE_PATH=$CMAKE_INCLUDE_PATH:/framework-dependencies/include"
executeCommand "source /root-system/bin/thisroot.sh"

executeCommand "rm -rf j-pet-framework || true"
executeCommand "git clone --single-branch --branch develop https://github.com/JPETTomography/j-pet-framework.git"
executeCommand "mkdir -p j-pet-framework/build"
executeCommand "cd j-pet-framework/build"
executeCommand "cmake .."
executeCommand "cmake --build ."
executeCommand "sudo make install"
executeCommand "cd ../.."

executeCommand "rm -rf build"
executeCommand "mkdir -p build"
executeCommand "cd build"
executeCommand "cmake .."
executeCommand "make"
executeCommand "make tests_largebarrel"
executeCommand "make tests_imagereconstruction"

executeCommand "cd ImageReconstruction"
executeCommand "ctest -j6 -C Debug -T test --output-on-failure"
executeCommand "cd .."

executeCommand "cd LargeBarrelAnalysis"
executeCommand "ctest -j6 -C Debug -T test --output-on-failure"
executeCommand "wget http://sphinx.if.uj.edu.pl/~alek/framework_integration_tests/dabc_17025151847.hld"
executeCommand "wget http://sphinx.if.uj.edu.pl/~alek/framework_integration_tests/setupRun3.json"
executeCommand "./LargeBarrelAnalysis.x -t hld -f dabc_17025151847.hld -l setupRun3.json -i 3 -r 0 100"
executeCommand "mkdir outdir"
executeCommand "./LargeBarrelAnalysis.x -t hld -f dabc_17025151847.hld -l setupRun3.json -i 3 -r 0 100 -o outdir"
executeCommand "./LargeBarrelAnalysis.x -t root -f dabc_17025151847.hld.root -l setupRun3.json -i 3 -r 0 100"
sed -i 's/manager.useTask(\"TimeWindowCreator\", \"hld\", \"tslot.calib\");//' ../../LargeBarrelAnalysis/main.cpp
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
executeCommand "make LargeBarrelAnalysis.x"
executeCommand "./LargeBarrelAnalysis.x -t root -f dabc_17025151847.tslot.calib.root -r 0 100"


if [ "$RUN_VELOCITY" = "1" ]; then
    executeCommand "cd ../VelocityCalibration/tests/unitTestData/VelocityCalibrationTest"
    executeCommand "rm -rf results_root_6"
    executeCommand "rm -rf results_root6.txt"
    executeCommand "rm -rf results.txt"
    executeCommand "mkdir -p results_root_6"
    executeCommand "rm -rf *.png"
    executeCommand "rm -rf calibrationRoot*"
    executeCommand "rm -rf Results"
    executeCommand "/usr/local/bin/run_velocity_calibration.sh"
    executeCommand "/usr/local/bin/compare_velocity_results.py calibrationRoot6EffVelocities.txt thresholdResults/EffVelocities.txt 5"
fi
