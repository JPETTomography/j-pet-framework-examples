# Scope Analysis

## Aim
This example shows an analysis of data from a two-strip J-PET prototype recorded with the Serial Data Analyzer (SDA, aka `Scope`).

## Input Data
This example uses the same dataset as the `ScopeLoaderExample`, located in the `ScopeLoaderExample/cfg/` directory. Note that the output files are also produced there.

## Expected output
No output to stdout.
`JPet_(date).log` file appears with the log of the processing and ROOT files with the following extensions are produced in `../ScopeLoaderExample/cfg/`:  
`*.reco.sig.root`  
`*.reco.sig.offsets`  
`*.reco.sig.charges`  
`*.reco.sig.charges.ampl`  
`*.phys.sig.root`  
`*.phys.hit.root`  
`*.phys.lor.root`  

where `*` stands for the name of the input location, i.e. each directory listed in the configuration file `../ScopeLoaderExample/cfg/example.json`

## Description
This example extends the `ScopeLoaderExample` with further processing of the data produced by ScopeLoader with several dedicated analysis modules. Code of the modules and their corresponding `tools` classes can be found in `modules/SDA` and `modules/tools` respectively.

The tasks performed on the data are:
 * calculation of signal level offsets
 * calculation of signals' charges
 * calculation of signals' amplitudes
 * drawing of the charge and amplitude spectra for the signals
 * assembling hits from the signals
 * assembling lines-of-response (LOR-s) from the hits (the resulting LOR-s can be further used for 2D image reconstruction)

## Additional info
Please refer to [README](../ScopeLoaderExample/README.md) for description of configuration file format.

## Compiling
`make`

## Running
Refer to the provided script `run.sh` for simple execution example, or execute this script directly.

## Author
Szymon Niedźwiecki  
Please report any bugs and suggestions of corrections to: [szymonniedzwiecki@googlemail.com](szymonniedzwiecki@googlemail.com)