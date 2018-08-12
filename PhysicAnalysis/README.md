# Physics stream

## Aim
This example show a simple procedure of Physics Analysis, for data collected by the Big Barrel.

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
For this example, the user must provide her/his own data file(s) collected with the Big Barrel.

## Description
The analysis extends the example of Large Barrel Analysis and substitutes Event Categorizer with some more detailed classification procedures.

## Additional info
For description of possible parameters, that can be used in `useParams.json`, see [PARAMETERS](PARAMETERS) file

## Compiling
`make`

## Running
Run in the similar way as the other examples.

## Authors
Kamil Dulski, Elena Pérez del Río
Please report any bugs and suggestions of corrections to [kamil.dulski@gmail.com](kamil.dulski@gmail.com)
