#!/bin/bash

# $1 directory with .list files, $2 common output dirirectory
# .list file should have the full paths to the files in HLD format

userFile="/absolute/path/to/user_params.json"
setupFile="/absolute/path/to/detector_configuration.json"

echo " "
echo "****"

if [ ! -z "$1" ] && [ ! -z "$2" ]
then

  for list in $1/*.list ;
  do
    echo " ";
    echo "****";
    date_time="`date +%Y-%m-%d_%H:%M:%S`";
    echo $date_time;
    echo "Processing files from list "$list;

    while IFS= read -r file
    do
      # Run the analysis of a single file in the background
      /data/3/users/kkacprzak/modular-framework/examples_build/ModularDetectorAnalysis/ModularDetectorAnalysis.x -t hld -k mod -i 38 -l $setupFile -u $userFile -d -f $file -o $2 &
    done < "$list"

    # Wait for programs in background to finish
    wait
  done # Finish iterating lists

  echo "Copying histograms..."
  for file in $2/*.cat.evt.root ; do
    name="${file%%.cat.evt.root}"
    echo $name
    rootcp $name".cat.evt.root:*/*" $name".C.root"
  done

  echo "HADDing files."
  rm $2/cat.root
  hadd $2/cat.root $2/*C.root

  echo "Cleaning..."
  rm $2/*.C.root

  echo "All done."
fi
