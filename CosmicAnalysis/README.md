# Cosmics stream - data selection of cosmic radiation events

## Aim
This example show a simple analysis of particles from cosmics radiation - data collected with the Big Barrel.

## Expected output
No output to stdout.
`JPet_(date).log` file appears with the log of the processing and ROOT files with the following extensions are produced:  
`*.tslot.calib.root`  
`*.raw.sig.root`  
`*.phys.sig.root`  
`*.hit.root`  
`*.unk.evt.root`  
`*.cosm.evt.root`  

where `*` stands for the name of the input file.

## Input Data
For this example, the user must provide her/his own data file(s) collected with the Big Barrel. Moreover, useful files with configurations and calibrations are downloaded during the `cmake` build to `CalibrationFiles` folder in the source folder.

## Description
The analysis extends the example of Large Barrel Analysis and substitutes Event Categorizer with version designed for cosmic radiation particles detection.

## Additional info
For description of possible parameters, that can be used in `userParams.json`, see [PARAMETERS](PARAMETERS.md) file

## Compiling
`make`

## Running
Run in the similar way as the LargeBarrelAnalysis example.

## Authors
Kamil Dulski, Elena Pérez del Río
Please report any bugs and suggestions of corrections to [kamil.dulski@gmail.com](kamil.dulski@gmail.com)
