# Large Barrel Analysis

## Aim
This example show a full analysis of data from the J-PET Big Barrel, which is so far the best attempt for J-PET data reconstruction.

## Expected output
No output to stdout.
`JPet_**.log` file appears with the log of the processing and ROOT files with the following extensions are produced:  
`*.tslot.calib.root`  
`*.raw.sig.root`  
`*.phys.sig.root`  
`*.hit.root`  
`*.unk.evt.root`  
`*.cat.evt.root`  

where `*` stands for the name of the input file.

## Input Data
For this example, user must provide their own data file(s) collected with the Big Barrel.

## Description
The analysis is split into tasks.

## Additional info
For description of possible parameters, that can be ised in `useParams.json`, see file [PARAMETERS](PARAMETERS).

## Compiling
`make`

## Running
The script `run.sh` contains an example of running the analysis. Note, however, that the user must fill the input data file name and the number of run.

## Authors
[Aleksander Gajos](https://github.com/alekgajos), [Krzysztof Kacprzak](https://github.com/kkacprzak), Nikodem Krawczyk  
Please report any bugs and suggestions of corrections to:  
[alek.gajos@gmail.com](alek.gajos@gmail.com)  
[k.kacprzak@yahoo.com](k.kacprzak@yahoo.com)
