# Scope Loader

## Aim
This example shows basic analysis use of `JPetScopeLoader`. Oscilloscope `ASCII` data files are converted into root files.

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

### Author:
Damian Trybek [damian.trybek@uj.edu.pl](damian.trybek@uj.edu.pl)  
Updated by Wojciech Krzemien [wojciech.krzemien@ncbj.gov.pl](wojciech.krzemien@ncbj.gov.pl)