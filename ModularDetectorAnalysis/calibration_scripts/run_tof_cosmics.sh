#!/bin/bash

# Variables defifnitions
userParams="/path/to/userParams.json"
calibFile="/path/to/calibration.json"
outputDir="/path/where/to/save/png/files/"

for i in 1 2 3 4 5 6 7 8 9 10
do
  echo "   "
  echo "***********"
  echo "Loop $i..."
  echo "***********"

  # Run program for each file
  echo "Running files..."
  for file in *.mtx.sig.root ;
  do
    /home/kkacprzak/3work/modular-framework/examples-build/ModularDetectorAnalysis/ModularDetectorAnalysis.x -t root -u $userParams -f $file -d &
  done

  # Wait for programs in background to finish
  wait

  # Copying histograms
  echo "Copying histograms..."
  for file in  *.cat.evt.root ;
  do
    rootcp $file":*/*" $file".C.root"
  done

  # Combining resulting files
  echo "HADDing..."
  hadd cat.root *.C.root

  # Move result, clean intermiediate files
  mv cat.root $outputDir
  rm *.C.root

  # Running ROOT macro
  echo "Run TOF synchronization for cosmic measurements..."
  root -b -l -q cosmic_tof.C'("'$outputDir'/cat.root", "'$calibFile'", true, "'$outputDir'", '$i')'

done

echo "All done."
