Aim
---
This is an example analysis producing J-PET time calibration using data taken with a reference detector.

Input Data
-----------
For a single execution of the program, a HLD file should be used, recorded when a single scintillator strip of the J-PET barrel was targeted by the reference detector.

Expected output
---------------
No output to stdout.
JPet.log file appears with the log of the processing and ROOT files with the following etensions are produced:
 *.tslot.raw.root
 *.tslot.calib.root
 *.raw.sig.root
 *.phys.sig.root
 *.hits.root
 *.calib.root
where "*" stands for the name of the input file.

Moreover, a text file "TimeConstantsCalib.txt" is created in the working directory, which contains time calibration information.

Description
--------------
The analysis is using the same set of tasks as the "LargeBarrelAnalysis" example up to the level of creation of hits (included). On top of these tasks, an additional module called "TimeCalibration" is run, which prepares spectra of time differences between signals on the left and fight side of a scintillator and determines the necessary shifts of times at respective TDC channels.

Additional info
--------------
In order to speed up the processing of a single position from a reference detector scan, one can provide a user option indicating which scintillator strip was targeted by the reference detector. This was, signals from any other strips will be ignored at the lowest possible level, resulting in much faster processing.

This user option is:
"TimeWindowCreator_MainStrip_int" : XYZ

where XYZ is a 3-digit number where
X - indicates the number of layer of the selected strip
YZ - indicates the number of the selected strip within its layer

In other words, the value of this option should be:
100 * (layer number) + (slot number)

Compiling 
------------
make

Running
------------
The best to run is to use a bash macro run_calibAll.sh which one can run with a list of paths to files containing data measured with refference detector for each position.
The file shoud have a specific format: LayerNumber_ScintillatorNumber: FolderWithFilesForScintillator (Scintillator number goes from 1-48 for layer 1 and 2 and  from 1-192
for layer 3). The file containing the list of filest should be given as the 4th argument of the macro. The First three arguments are the first and last slot which we calibrate
and layer which we calibrate. The other arguments are: the path where You store the data, first file which we calibrate per a given position, last file which we calibrate per
a given position (since we could have more than ne hld file mesured pre position), file with TOT correction and number of running period (depending on it we use different json
file with detector configuration and different run number.
The example of running macro:
run_calibAll.sh 3 5 3 FileListRun2.txt /media/silarski/backup/JPETRefII_data/L3/ 1 1 TOT_run1_2.root 2

Note that with this macro You can run directly on the xz files and the TimeWindowCreator_MainStrip_int option in the userparams.json is modified using sed according to the
file we analyze.

The single run of the module looks like:
./TimeCalibration_dev.x -t hld -f FileName.hld -i RunJson -c TOTFile -u UserParamsFile

Author
------------
Magdalena Skurzok and Michał Silarski
Please report any bugs and suggestions of corrections to: <michal.silarski@uj.edu.pl>
