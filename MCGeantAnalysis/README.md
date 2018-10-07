# MCGeantAnalysis

## Aim
Module is used to load a simulation files created with [J-PET-geant4](https://github.com/JPETTomography/J-PET-geant4). 

## Input and Output
Input file must have the `*.mcGeant.root`  
Also required is a `.json` with simulated geometry
Output file created   `*.mc.hits.root`  
No output to stdout,`JPet_(date).log` file appears with the log of the processing and ROOT files.

## How to use?
`./run.sh *.mcGeant.root`

## Implemented features
Detector geometry is created based on `detectorSetup.json` file provided by the user
