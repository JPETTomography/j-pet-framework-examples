# Physics stream parameters description

## Contents
Description of available parameters in Physics example. Note that configuration/calibration files can be obtained from [PetWiki](http://koza.if.uj.edu.pl/petwiki/index.php/Default_settings_and_parameters_used_in_the_analyses)

- `Save_Control_Histograms_bool`  
Common for each module, if set to `true`, in the output `ROOT` files folder with statistics will contain control histograms. Set to `false` if histograms not needed.

- `TimeWindowCreator_MainStrip_int`  
Dedicated for files with data from measurement with Reference Detector - it denotes which `ID` has the reference scintillator. If option not set, part with reference detector is ignored

- `TimeWindowCreator_MinTime_float`  
Time Slots have certain duration for specific runs, minimum time is usually negative, default value `-1*10^6 ps`

- `TimeWindowCreator_MaxTime_float`  
default value `0.0 ps`

- `TimeCalibLoader_ConfigFile_std::string`  
name of ASCII file of required structure, containing time calibrations, specific for each run

- `SignalFinder_EdgeMaxTime_float`  
time window for matching Signal Channels on Leading Edge. Default value: `5 000 ps`

- `SignalFinder_LeadTrailMaxTime_float`  
time window for matching Signal Channels on the same thresholds from Leading and Trailing edge. Default value: `25 000 ps`

- `HitFinder_VelocityFile_std::string`  
name of ASCII file of required format, containing values of effective velocities of light in each scintillator

- `HitFinder_ABTimeDiff_float`  
time window for matching Signals on the same scintillator and different sides. Default value: `6 000 ps`

- `HitFinder_RefDetScinID_int`  
`ID` of Reference Detector Scintillator, needed for creating reference hits

- `HitFinder_TOTCalculationType_std::string`  
Type of the calculations of the TOT - it can be standard sum (option "standard"), a extended sum taking into account thresholds differences and calculated as rectangulars (option "rectangular"), additional extension that add also differences between the TOTs on different thresholds and calculates sum as sum of the trapezes (option "trapeze")

- `EventFinder_EventTime_float`  
time window for grouping hits in one event. Default value `5 000 ps`

- `EventFinder_MinEventMultiplicity_int`  
events of minimum multiplicity will only be saved in output file. Default value is 1, so all events are saved.

- `EventCategorizer_BackToBackAngleWindow_float`  
denotes acceptable difference in degrees between opposite slots, to categorize two hits as back-to-back type. Default value is `3.0` degrees, so accepted slot theta difference will be between `177.0` and `183.0` degrees.

- `EventCategorizer_MinDeexcitationTOT_float`  
denotes `Time over Threshold` cut minimal value for simple selection of deexcitation photons. Default value: `30 000 ps`.

- `EventCategorizer_MaxDeexcitationTOT_float`  
denotes Time over Threshold cut maximal value for simple selection of deexcitation photons. Default value: `50 000 ps`.

- `EventCategorizer_MinAnnihilationTOT_float`  
denotes `Time over Threshold` cut minimal value for simple selection of annihilation photons. Default value: `10 000 ps`.

- `EventCategorizer_MaxAnnihilationTOT_float`  
denotes `Time over Threshold` cut maximal value for simple selection of annihilation photons. Default value: `25 000 ps`.

- `EventCategorizer_MaxDistOfDecayPlaneFromCenter_float`  
denotes the maximum distance in `[cm]` of the plane, that is created by 3 hits, from the center of the detector - point `(0,0,0)`. Default value: `5 cm`.

- `EventCategorizer_DecayInto3MinAngle_float`  
value in deg, that denotes a cut on 3 relative angles transformed histogram, that is used in the technique of selecting o-Ps candidates in the reconstruction. Default value: `190.0 deg`

- `EventCategorizer_MaxTimeDiff_float`  
maximum time difference between first and the last hit in an event, that can be used in selecting various events. Default value: `1000 ps`.

- `EventCategorizer_MaxHitZPos_float`  
cut on `z-axis` position of a hit in the scintillator in `[cm]`. Default value: `23 cm`, so accepted hits will have `z` position between `-23` and `23` `cm`.
