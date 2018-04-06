MCGeantAnalysis
=============================================================================

## Basic information
Module is used to load a simulation files created with JPetMC code (https://github.com/daria137/jpetmc.git).
Input files has to have NAME.mcGeant.root extention.
Created output files names follows pattern: NAME.mc.hits.root

## How to use?
./run.sh NAME.mcGeant.root

Required files:
- .json (with simulated geometry details)
- necessary calibration files 


## Implemented features
- detector geometry is created based on detectorSetup.json file provided by user

## TODO

