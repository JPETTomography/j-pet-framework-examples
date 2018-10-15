# Time Calibration

## Aim
This is an example analysis producing J-PET time calibration using data taken with a reference detector.

## Input Data
For a single execution of the program, a HLD file should be used, recorded when a single scintillator strip of the J-PET barrel was targeted by the reference detector.

## Expected output
No output to stdout.
`JPet_(date).log` file appears with the log of the processing and ROOT files with the following extensions are produced:  
`*.tslot.raw.root`  
`*.tslot.calib.root`  
`*.raw.sig.root`  
`*.phys.sig.root`  
`*.hits.root`  
`*.calib.root`  

where `*` stands for the name of the input file. Moreover, a text file `TimeConstantsCalib.txt` is created in the working directory, which contains time calibration information.

## Description
The analysis is using the same set of tasks as the `LargeBarrelAnalysis` example up to the level of creation of hits (included). On top of these tasks, an additional module called `TimeCalibration` is run, which prepares spectra of time differences between signals on the left and fight side of a scintillator and determines the necessary shifts of times at respective TDC channels.

## Additional info
In order to speed up the processing of a single position from a reference detector scan, one can provide a user option indicating which scintillator strip was targeted by the reference detector. This was, signals from any other strips will be ignored at the lowest possible level, resulting in much faster processing.

This user option is:
`"TimeWindowCreator_MainStrip_int" : XYZ`  
where XYZ is a 3-digit number where  
`X` - indicates the number of layer of the selected strip  
`YZ` - indicates the number of the selected strip within its layer  
In other words, the value of this option should be: `100 * (layer number) + (slot number)`

## Compiling
`make`

## Running
Please contact the Authors for details of usage.

## Authors
Magdalena Skurzok and Michał Silarski
Please report any bugs and suggestions of corrections to: [michal.silarski@uj.edu.pl](michal.silarski@uj.edu.pl)