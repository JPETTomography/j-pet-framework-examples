# Large Barrel Analysis parameters description

## Contents
Description of available parameters in LargeBarrelAnalysis example. Note that configuration/calibration files can be obtained from [PetWiki](http://koza.if.uj.edu.pl/petwiki/index.php/Default_settings_and_parameters_used_in_the_analyses)

- `Save_Control_Histograms_bool`  
Common for each module, if set to `true`, in the output `ROOT` files folder with statistics will contain control histograms. Set to `false` if histograms are not needed.

- `Unpacker_TOToffsetCalib_std::string`  
Path to and name of a `ROOT` file with `TOT` offset calibrations (stretcher) applied during unpacking of `HLD` file.

- `Unpacker_TDCnonlinearityCalib_std::string`  
Path to and name of a `ROOT` file with calibrations of nonlinearities occurring on `TDC` boards, applied during unpacking of `HLD` file.

- `TimeWindowCreator_MainStrip_int`  
Dedicated for files with data from measurement with Reference Detector - it denotes which `ID` has the scintillator we calibrate. If option is absent in the user parameters file, part with reference detector is ignored

- `TimeWindowCreator_MinTime_float`  
Time Slots have certain duration for specific runs, minimum time is usually negative, default value `-1*10^6 ps`

- `TimeWindowCreator_MaxTime_float`  
default value `0.0 ps`

- `TimeCalibLoader_ConfigFile_std::string`  
Path to and name of ASCII file of required structure, containing time calibrations, specific for each run

- `SignalFinder_UseCorruptedSigCh_bool`  
Indication if Signal Finder module should use signal channels flagged as Corrupted in the previous task. Default value: `false`

- `SignalFinder_EdgeMaxTime_float`  
time window for matching Signal Channels on Leading Edge. Default value: `5 000 ps`

- `SignalFinder_LeadTrailMaxTime_float`  
time window for matching Signal Channels on the same thresholds from Leading and Trailing edge. Default value: `25 000 ps`

- `SignalTransformer_UseCorruptedSignals_bool`  
Indication if Signal Transformer module should use signals flagged as Corrupted in the previous task. Default value: `false`

- `SignalTransformer_WalkCorrConstThr1_float`
Constant used to calculate the walk correction for threshold 1 (on both edges), Default value is 0.0

- `SignalTransformer_WalkCorrConstThr2_float`
Constant used to calculate the walk correction for threshold 2 (on both edges), Default	value is 0.0

- `SignalTransformer_WalkCorrConstThr3_float`
Constant used to calculate the walk correction for threshold 3 (on both edges), Default	value is 0.0

- `SignalTransformer_WalkCorrConstThr4_float`
Constant used to calculate the walk correction for threshold 4 (on both edges),	Default value is 0.0

- `HitFinder_UseCorruptedSignals_bool`  
Indication if Hit Finder module should use signals flagged as Corrupted in the previous task. Default value: `false`

- `HitFinder_VelocityFile_std::string`  
Path to and name of ASCII file of required format, containing values of effective velocities of light in each scintillator

- `HitFinder_ABTimeDiff_float`  
time window for matching Signals on the same scintillator and different sides. Default value: `6 000 ps`

- `HitFinder_RefDetScinID_int`  
`ID` of Reference Detector Scintillator, needed for creating reference hits

- `EventFinder_UseCorruptedHits_bool`  
Indication if Event Finder module should use hits flagged as Corrupted in the previous task. Default value: `false`

- `EventFinder_EventTime_float`  
time window for grouping hits in one event. Default value `5 000 ps`

- `EventFinder_MinEventMultiplicity_int`  
events of minimum multiplicity will only be saved in output file. Default value is 1, so all events are saved.

- `Scatter_Categorizer_TOF_TimeDiff_float`  
categorizer tool for recognizing scatterings. User can constrain allowed discrepancy between calculated time of flight of scatter candidate and difference of two hit times. Default value `2000 ps`

- `Back2Back_Categorizer_SlotThetaDiff_float`  
denotes acceptable difference in degrees between opposite slots, to categorize two hits as back-to-back type. Default value is `3.0` degrees, so accepted slot theta difference will be between `177.0` and `183.0` degrees.

- `Deex_Categorizer_TOT_Cut_Min_float`  
denotes Time over Threshold cut minimal value for simple selection of deexcitation photons Default value: `30 000 ps`

- `Deex_Categorizer_TOT_Cut_Max_float`  
denotes Time over Threshold cut maximal value for simple selection of deexcitation photons. Default value: `50 000 ps`

- TOT to energy conversion parameters:  
`ToTEnergyConverterFactory_ToT2EnergyFunction_std::string`  
String with function formula in ROOT format  
See documentation of [TFormula class](https://root.cern.ch/doc/master/classTFormula.html)
`ToTEnergyConverterFactory_ToT2EnergyParameters_std::vector<double>`  
Array of parameters for function above, given as a vector of doubles  
`ToTEnergyConverterFactory_ToT2EnergyFunctionLimits_std::vector<double>`  
Range of convertible TOT values, also a vector of doubles, consisting of 2 elements  
