# Velocity Calibration

## Aim
This module is dedicated for the measurements with the collimator in the different positions along the z axis to estimate the effective velocity of the signals inside the scintillators.

## Expected output
No output to stdout.
`JPet_(date).log` file appears with the log of the processing and ROOT files with the following extensions are produced:  
`*.tslot.calib.root`  
`*.raw.sig.root`  
`*.phys.sig.root`  
`*.hits.root`  
`*.deltaT.root`  

where `*` stands for the name of the input file.

## Input Data
For this example, the user must provide her/his own data file(s) collected with the Big Barrel with he collimator placed in the different positions along the z axis.

## Description
This module is looking for the time difference between A and B side for each scintillator. A Gaussian function is fitted to each time difference A-B (algorithm is looking for maximum bin and Gaussian function is fitted in the range of the maximum bin +/- 2 bins). Control histograms are saved to the automatically created folder `Results`. A `txt` file with the fit results is also created. The parameters: `slot_id`, `position`, `threshold`, `mean value` of fitted function, `error` of fitted function, `chi_square` and `NDF` are saved to the file with the results.

The user can put in the `userParams.json` file the position and corresponding to this position file name, which will be analyzed by Framework. `DeltaTFinder` module compares the name of the file given from command line with the name of the file in the `userParams.json` and based on this comparison, `DeltaTFinder` saves a proper position to the results file. 

`userParams` options:

`Save_Cotrol_Histograms_bool` - Common for each module, if set to `true`, in the output `ROOT` files folder with statistics will contain control histograms. Set to `false` if histograms not needed;

`Unpacker_TOToffsetCalib_std::string` - Path to and name of a ROOT file with TOT offset calibrations (stretcher) applied during unpacking of HLD file;

`Unpacker_TDCnonlinearityCalib_std::string` - Path to and name of a `ROOT` file with calibrations of nonlinearities occurring on `TDC` boards, applied during unpacking of `HLD` file.

`TimeWindowCreator_MinTime_float` - Time Slots have certain duration for specific runs, minimum time is usually negative, default value `-1*10^6 ps`

`TimeWindowCreator_MaxTime_float` - default value `0.0 ps`

`TimeCalibLoader_ConfigFile_std::string` - Path to and name of ASCII file of required structure, containing time calibrations, specific for each run

`ThresholdLoader_ConfigFile_std::string` - Path to and name of ASCII file of required structure, containing thresholds values, specific for each run

`SignalFinder_UseCorruptedSigCh_bool` - if set to `true`, signal channels flagged as corrupted will be used in SignalFinder analysis, if set to `false` they will be filtered out

`SignalFinder_EdgeMaxTime_float` - time window for matching Signal Channels on Leading Edge. Default value: `5 000 ps`

`SignalFinder_LeadTrailMaxTime_float` - time window for matching Signal Channels on the same thresholds from Leading and Trailing edge. Default value: `25 000 ps`

`SignalTransformer_UseCorruptedSignals_bool` - if set to `true`, signals flagged as corrupted will be used in SignalTransformer analysis, if set to `false` they will be filtered out

`HitFinder_UseCorruptedSignals_bool` - if set to `true`, signals flagged as corrupted will be used in HitFinder analysis, if set to `false` they will be filtered out

`HitFinder_VelocityFile_std::string` - name of ASCII file of required format, containing values of effective velocities of light in each scintillator

`HitFinder_ABTimeDiff_float` - time window for matching Signals on the same scintillator and different sides. Default value: `6 000 ps`

`HitFinder_RefDetScinID_int` - `ID` of Reference Detector Scintillator, needed for creating reference hits

`DeltaTFinder_numberOfPositions_std::string` - number of positions used in analysis
  
`DeltaTFinder_Position_?_std::string"` - declaration of correspondence between position in [mm] and file name, file name should be provided without full path and extensions (i.e. `dabc_17207020324` not `/path/to/file/dabc_17207020324.hits.root`), in place of `?` a int number should be placed within range from 1 to maximum of `DeltaTFinder_numberOfPositions_std::string` flag value

`DeltaTFinder_outputPath_std::string` - path to folder in which control spectra will be saved, for empty path the folder with control spectra with be created in the same folder as program  

`DeltaTFinder_velocityCalibFile_std::string` - file name with results which will be created at the end of analysis, which later has to be provided to estimateVelocity program

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
