# InterThresholdCalibration parameters description

## Contents
Description of available parameters in InterThresholdCalibration example. Note that configuration/calibration files can be obtained from [PetWiki](http://koza.if.uj.edu.pl/petwiki/index.php/Default_settings_and_parameters_used_in_the_analyses)
This example uses modules from LargeBarrelAnalysis and the parameters defined there can be used.

- `Save_Control_Histograms_bool`  
Common for each module, if set to `true`, in the output `ROOT` files folder with statistics will contain control histograms. Set to `false` if histograms not needed.

- `InterThresholdCalibration_TimeCalibRunNumber_int`  
Number of calibration run associated with Acquisition Campaign. Default value: `4`

- `InterThresholdCalibration_Frac_err_double`  
maximal fractional uncertainty of parameters accepted by calibration. Default value: `0.3`

- `InterThresholdCalibration_Min_ev_int`  
minimal number of events for a distribution to be fitted. Default value: `100`

- `InterThresholdCalibration_TimeConstantsInterThrCalibOutputFile_std::string`
name and path to the output file produced by calibration
