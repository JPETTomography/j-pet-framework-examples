# Velocity Calibration

## Aim
This module is dedicated for the measurements with the collimator in the different positions along the z axis to estimate the effective velocity of the signals inside the scintillators.

## Expected output
No output to stdout.
`JPet_**.log` file appears with the log of the processing and ROOT files with the following extensions are produced:  
`*.tslot.calib.root`  
`*.raw.sig.root`  
`*.phys.sig.root`  
`*.hit.root`  
`*.deltaT.root`  

where `*` stands for the name of the input file.

## Input Data
For this example, the user must provide her/his own data file(s) collected with the Big Barrel with he collimator placed in the different positions along the z axis.

## Description
This module is looking for the time difference between A and B side for each scintillator. A Gaussian function is fitted to each time difference A-B (algorithm is looking for maximum bin and Gaussian function is fitted in the range of the maximum bin +/- 2 bins). Control histograms are saved to the automatically created folder `Results`. A `txt` file with the fit results is also created. The parameters: `slot_id`, `position`, `threshold`, `mean value` of fitted function, `error` of fitted function, `chi_square` and `NDF` are saved to the file with the results.

The user can put in the `userParams.json` file the position and corresponding to this position file name, which will be analyzed by Framework. `DeltaTFinder` module compares the name of the file given from command line with the name of the file in the `userParams.json` and based on this comparison, `DeltaTFinder` saves a proper position to the results file. There is no need to write magic numbers inside code of `DeltaTFinder`.

`userParams` options:

`"localDB":"setupRun2NoRef.json"` - local database  
`"TimeCalibLoader_ConfigFile":"TimeConstantsCollimator.txt"` - this is not used because this module does not use the second task  
`"DeltaTFinder_numberOfPositions": "2"` - number of position you want to specify  
`"DeltaTFinder_Position1":"0 /path/to/file/dabc_17207043432.hits.root"` - exemplary position and path to one file (the same path must be provided while program running)  
`"DeltaTFinder_Position2":"50 /path/to/file/dabc_17207020324.hits.root"` - exemplary position and path to second file (the same path must be provided while program running)  
`"DeltaTFinder_outputPath":""` - path to folder in which control spectra will be saved, for empty path the folder with control spectra with be created in the same folder as program  
`"DeltaTFinder_velocityCalibFile":"results.txt"` - file name with results which will be created at the end

Another, separate program (`estimateVelocity.cpp`) has been written to estimate effective velocity of signal inside the scintillator based on file `results.txt`. This program draws the dependence between the position and the mean value of Gaussian function for a given scintillator. Later a polynomial (pol1) function is fitted to this points and p1 parameter of this function is treated as a effective velocity of signal.

## Additional info
To run `VelocityCalibration` module properly one has to create velocity calibration file with `0` value as the effective velocity of light inside the scintillator which is used by `HitFinder`. This file is read in `HitFinder` and it was not possible to turn off loading of previous effective velocity calibration.

## Compiling
`make`

## Running
The script `run.sh` contains an example of running the analysis for two files one after another. Note, however, that the user must fill the input data file name and the number of run as well as take care when setting the calibration of times and velocity (see additional info section above).

## Author
Monika Pawlik-Niedźwiecka
Please report any bugs and suggestions of corrections to: [monikapawlik88@gmail.com](monikapawlik88@gmail.com)
