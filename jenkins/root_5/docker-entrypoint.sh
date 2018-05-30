mkdir -p build
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd build
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
export CMAKE_LIBRARY_PATH=$CMAKE_LIBRARY_PATH:/framework-dependencies/lib
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
export CMAKE_INCLUDE_PATH=$CMAKE_INCLUDE_PATH:/framework-dependencies/include
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
source /root-system/bin/thisroot.sh
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cmake ..
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd LargeBarrelAnalysis
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
wget http://sphinx.if.uj.edu.pl/~alek/framework_integration_tests/dabc_17025151847.hld
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
wget http://sphinx.if.uj.edu.pl/~alek/framework_integration_tests/setupRun3.json
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
./LargeBarrelAnalysis.x -t hld -f dabc_17025151847.hld -l setupRun3.json -i 3 -r 0 100
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
mkdir outdir
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
./LargeBarrelAnalysis.x -t hld -f dabc_17025151847.hld -l setupRun3.json -i 3 -r 0 100 -o outdir
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
./LargeBarrelAnalysis.x -t root -f dabc_17025151847.hld.root -l setupRun3.json -i 3 -r 0 100
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
sed -i 's/manager.useTask("TimeWindowCreator", "hld", "tslot.raw");//' ../../LargeBarrelAnalysis/main.cpp
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
sed -i 's/manager.useTask("TimeCalibLoader", "tslot.raw", "tslot.calib");//' ../../LargeBarrelAnalysis/main.cpp
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
./LargeBarrelAnalysis.x -t root -f dabc_17025151847.tslot.calib.root   -r 0 100
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../TimeCalibration
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../VelocityCalibration
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
cd ../NewAnalysisTemplate
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
make
rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi
