# Scope Loader

## Aim
This example shows basic analysis use of `JPetScopeLoader`. Oscilloscope `ASCII` data files are converted into root files. 
ASCII files have two collumns, separated by white sign. Each row contains information from signal probing. First collumn stores time of probing in `[s]` 
while second one has voltage value stored in `[mV]`. 
Program assumes that:
* first 5 lines of each file are describtion from scope and will be omitted;
* each signal is stored in separate file;
* file names follow convention `C?_*****.txt`, where `?` is channel number, ranging from 1 to 4, and `*****` denotes time window;
* each time window contains four signals (i.e. `C1_00000.txt`, `C2_00000.txt`, `C3_00000.txt` `C4_00000.txt`); 
* files are grouped in directories named by integer numbers, corresponding usually to position of irradiation;

## Running
`./ScopeReaderExample.x -t scope -f ./cfg/example.json -l ./cfg/example_params.json -i 1`  
where:  
`-t scope` sets input file type to oscilloscpe ASCII data  
`-f ./cfg/example` sets location of configuration file with positions  
`-l ./cfg/example_params.json` sets location of the configuration file with setup settings  
`-i 1` is a run number corresponding to the detector setting read from example_params.json  

## Expected output:
 * log file `JPet_(date).log`
 * set of root files (one root file per collimator position) in the `cfg` folder in the following format: `example.@@.reco.sig.root` where `@@` stands for collimator position (eg. `example_config1_1.reco.sig.root`).

## Additional info:
Structure of the configuration json file:
```
{
  "config1":
    {
      "location": "data",
      "collimator":
        [
          {
            "positions": "1 5 2"
          },
          {
            "positions": "12"
          },
          {
            "positions": "6"
          }
        ]
     }
}
```

* `location` - path to directory with data directories;
* `positions` - either one directory from location field, which will be loaded or a range set with convention START, STOP, STEP, in case of above example 
positions 1, 3 and 5 will be processed

Exemplary data structure is presented in `cfg` directory.

### Author:
Damian Trybek [damian.trybek@uj.edu.pl](damian.trybek@uj.edu.pl)  
Updated by Wojciech Krzemien [wojciech.krzemien@ncbj.gov.pl](wojciech.krzemien@ncbj.gov.pl)
