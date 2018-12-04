# Interthreshold Time Calibration

## Aim
This is an example of analysis, producing J-PET interthreshold time calibration using data taken in dedicated measurement, in which one threshold of chosen PMT was distinguished and three others were set to match their counts with the chosen one.

## Expected output
No output to stdout.
`JPet_(date).log` file appears with the log of the processing and ROOT files with the following extensions are produced:  
`*.tslot.calib.root`  
`*.raw.sig.root`  
`*.phys.sig.root`  
`*.hits.root`  
`*.hits.calib.root`  

where `*` stands for the name of the input file.

## Input Data
For this example, user must provide data file(s) collected with the collimator at position `z=0` in measurement dedicated for interthreshold calibration.

## Description
The analysis is using the same set of tasks as the `LargeBarrelAnalysis` example up to the level of creation of hits (included). On top of these tasks, an additional module called `InterThresholdCalibration` is run, which prepares spectra of time differences between signals on threshold 1 (`A` - lowest) and other thresholds, separately on side `A` and `B` of each scintillator.

## Additional info
A text file `TimeConstantsInterThrCalib.txt` is created in the working directory, which contains time calibration information. For description of possible parameters, that can be used in `useParams.json`,
see [PARAMETERS](PARAMETERS.md) file. Take time callibration with TDC correction for dedicated RUN!

## Compiling
`make`

## Running
The script `run.sh` contains an example of running the analysis. Note, however, that the user must fill the input data file name and the number of run.

## Authors
Magdalena Skurzok and Szymon Niedzwiecki  
Please report any bugs and suggestions of corrections to: [mskurzok@gmail.com](mskurzok@gmail.com)
