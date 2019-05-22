# InterThresholdCalibration parameters description

## Contents
Description of available parameters in InterThresholdCalibration example. Note that configuration/calibration files can be obtained from [PetWiki](http://koza.if.uj.edu.pl/petwiki/index.php/Default_settings_and_parameters_used_in_the_analyses)

- `Save_Cotrol_Histograms_bool`  
Common for each module, if set to `true`, in the output `ROOT` files folder with statistics will contain control histograms. Set to `false` if histograms not needed.

- `Unpacker_TOToffsetCalib_std::string`  
Name of a `ROOT` file with `TOT` offset calibrations (stretcher) applied during unpacking of `HLD` file.

- `Unpacker_TDCnonlinearityCalib_std::string`  
Name of a `ROOT` file with calibrations of nonlinearities occurring on `TDC` boards, applied during unpacking of `HLD` file.

- `TimeWindowCreator_MainStrip_int`  
Dedicated for files with data from measurement with Reference Detector - it denotes which `ID` has the reference scintillator. If option not set, part with reference detector is ignored

- `TimeWindowCreator_MinTime_float`  
Time Slots have certain duration for specific runs, minimum time is usually negative, default value `-6.5*10^5 ps`

- `TimeWindowCreator_MaxTime_float`  
default value `0.0 ps`

- `TimeCalibLoader_ConfigFile_std::string`  
name of ASCII file of required structure, containing time calibrations, specific for each run

- `ThresholdLoader_ConfigFile_std::string`
name of an ASCII file containing thresholds set to the FEE channels, specific for each run

- `SignalFinder_UseCorruptedSigCh_bool`  
Indication if Signal Finder module should use signal channels flagged as Corrupted in the previous task. Default value: `false`

- `SignalFinder_EdgeMaxTime_float`  
time window for matching Signal Channels on Leading Edge. Default value: `5 000 ps`

- `SignalFinder_LeadTrailMaxTime_float`  
time window for matching Signal Channels on the same thresholds from Leading and Trailing edge. Default value: `23 000 ps`

- `SignalTransformer_UseCorruptedSignals_bool`  
Indication if Signal Transformer module should use signals flagged as Corrupted in the previous task. Default value: `false`

- `HitFinder_UseCorruptedSignals_bool`  
Indication if Hit Finder module should use signals flagged as Corrupted in the previous task. Default value: `false`

- `HitFinder_VelocityFile_std::string`  
name of ASCII file of required format, containing values of effective velocities of light in each scintillator

- `HitFinder_ABTimeDiff_float`  
time window for matching Signals on the same scintillator and different sides. Default value: `6 000 ps`

- `HitFinder_RefDetScinID_int`  
`ID` of Reference Detector Scintillator, needed for creating reference hits

- `InterThresholdCalibration_TimeCalibRunNumber_int`  
Number of calibration run associated with Acquisition Campaign. Default value: `4`

- `InterThresholdCalibration_Frac_err_double`  
maximal fractional uncertainty of parameters accepted by calibration. Default value: `0.3`

- `InterThresholdCalibration_Min_ev_int`  
minimal number of events for a distribution to be fitted. Default value: `100`

- `InterThresholdCalibration_TimeConstantsInterThrCalibOutputFile_std`
name and path to the output file produced by calibration
