Aim
---
This is an example analysis producing J-PET time calibration using data taken with a reference detector with option of iterative calibration constants determination. 

Input Data
-----------
For a single execution of the program, a HLD file should be used, recorded when a single scintillator strip of the J-PET barrel was targeted by the reference detector.

Expected output
---------------
No output to stdout.
JPet.log file appears with the log of the processing and ROOT files with the following etensions are produced:
 *.tslot.raw.root
 *.raw.sig.root
 *.phys.sig.root
 *.hits.root
 *.calib.root
where "*" stands for the name of the output file (constructed as LayerId_slotId_FileId, e.g. for the first file measured for first scintillator in the first layer: "Layer1_slot01_1")

Moreover, a text file "TimeConstantsCalib.txt" is created in the working directory, which contains time calibration information.
For the iterative version we create also temporary file "ScintId_LayerId_TimeConstantsCalibTmp.txt" where we save calibration constants after each iteration
(again for the first scintillator in the first layer the filename is 1_1_TimeConstantsCalibTmp.txt.)

Description
--------------
The analysis is using the same set of tasks as the "LargeBarrelAnalysis" example up to the level of creation of hits (included). On top of these tasks, an additional module called
"TimeCalibration" is run, which prepares spectra of time differences between signals on the left and fight side of a scintillator and determines the necessary shifts of
times at respective TDC channels.

Additional info
--------------
In order to speed up the processing of a single position from a reference detector scan, one can provide a user option indicating which scintillator strip was targeted by the reference detector.
This was, signals from any other strips will be ignored at the lowest possible level, resulting in much faster processing.

This user option is:
"TimeWindowCreator_MainStrip_int" : XYZ

where XYZ is a 3-digit number where
X - indicates the number of layer of the selected strip
YZ - indicates the number of the selected strip within its layer

In other words, the value of this option should be:
100 * (layer number) + (slot number)

The number of iterations can be defined in the main.cpp as a parameter in manager.useTask. The infinite number of iterations can be set by setting it to -1:
 manager.useTask("TimeCalibration", "hits", "calib",-1)
this allows one to define the condition of closing the iterative loop by a user defined condition. For now we defined only the maximum number of iterations
to be specified by the user. One needs do set the option "StopIteration_bool" to false and give the number of iterations by "TimeCalibration_NiterMax_int".
The other condition to stop the iterative calibration can be expressed in "TimeCalibration::CheckIfExit" function in the TimeCalibration_dev.cpp file.

Compiling 
------------
make

Running
------------
The best to run is to use a bash macro run_calibAll_iter.sh which one can run with a list of paths to files containing data measured with refference detector for each position.
The file shoud have a specific format: LayerNumber_ScintillatorNumber: FolderWithFilesForScintillator (Scintillator number goes from 1-48 for layer 1 and 2 and  from 1-192
for layer 3). The file containing the list of filest should be given as the 4th argument of the macro. The First three arguments are the first and last slot which we calibrate
and layer which we calibrate. The other arguments are: the path where You store the data, first file which we calibrate per a given position, last file which we calibrate per
a given position (since we could have more than ne hld file mesured pre position), number of running period (depending on it we use different json file with detector configuration
and different run number, for run 6 use 61 for A part, 62 for B part, 63 for C and 64 for D), type of the file to be analyzed (hld or root,if xz the macro will extrct the hld file
with hld flag) and otput directory for the final files.The userParams and DetectorSetup are defined as specific files for each run, e.g. for run 4 we have to use
the userParamsRun4.json and detectorSetupRun4.json. Here we assume that the detectorSetup, TOT, TDC nonlinearity are in CalibrationFiles/X_RUN/ as given from the Petwiki
IT IS THE USER WHO SHOULD PROVIDE PATH TO THESE FILES (if conditions below the variables definition). 
The Unzipper specifies the program (together with options) used  to unzip the files (xz and bz2 require different programs)
The example of running macro:
source run_calibAll_iter.sh 3 5 3 FileListRun2.txt /media/silarski/backup/JPETRefII_data/L3/ 1 2 hld /media/silarski/backup/

The single run of the module looks like:
./TimeCalibration_iter.x -t hld -f $NewFileName.$Tfile -i $RunJson -u $UserParams -b -o $OutDir p conf_trb3.xml -l $DetectorSetup

Author
------------
Magdalena Skurzok and Michał Silarski
Please report any bugs and suggestions of corrections to: <michal.silarski@uj.edu.pl>
