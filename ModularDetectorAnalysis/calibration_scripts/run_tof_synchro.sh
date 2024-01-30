#!/bin/bash

# Variables defifnitions
userFile="/absolute/path/to/user_params.json"
calibFile="/absolute/path/to/calibration_file.json"
setupFile="/absolute/path/to/detector_configuration.json"
inputDir="/absolute/path/to/folder_with_lists"
outputDir="/absolute/path/to/output_directory"

for i in 1 2 3 4 5 ; do
  echo "   "
  echo "***********"
  echo "Loop $i..."
  echo "***********"

  # Run files from lists in input directory
  for list in $inputDir/*.list ;
  do
    echo " "
    echo "****"
    date_time="`date +%Y-%m-%d_%H:%M:%S`";
    echo $date_time;
    echo "Processing files from list "$list

    while IFS= read -r file
    do
      #echo "$file"
      # Run the analysis of a single file in the background
      /data/3/users/kkacprzak/modular-framework/examples_build/ModularDetectorAnalysis/ModularDetectorAnalysis.x -t hld -k mod -i 38 -l $setupFile -u $userFile -d -f $file -o $outputDir &
    done < "$list"

    # Wait for programs in background to finish
    wait
  done # Finish iterating lists

  # Extracting histograms
  echo "Histograms - processing files from "$outputDir
  for file in $outputDir/*.cat.evt.root ; do
    name="${file%%.cat.evt.root}"
    echo $name
    rootcp $name".cat.evt.root:*/*" $name".C.root"
  done

  echo "HADDing files."

  rm $outputDir/cat.root
  hadd $outputDir/cat.root $outputDir/*.C.root

  echo "Cleaning..."

  rm $outputDir/*.C.root

  # Running ROOT macro
  echo "Run TOF synchronization..."
  root -b -l -q tof_synchro_it.C'("'$outputDir'/cat.root", "'$calibFile'", true, "'$outputDir'", '$i')'

done

echo "All done."
