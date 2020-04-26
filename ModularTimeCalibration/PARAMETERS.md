Description of available parameters in TimeCalibration example.
Note that configuration/calibration filed can be obtained from PetWiki:
http://koza.if.uj.edu.pl/petwiki/index.php/Default_settings_and_parameters_used_in_the_analyses
--------

Save_Cotrol_Histograms_bool
--- Common for each module, if set to true, in the output ROOT files folder with
statistics will contain control histograms. Set to false if histograms not needed.

TimeWindowCreator_MainStrip_int
--- Dedicated for files with data from measurement with Reference Detector:
In order to speed up the processing of a single position from a reference detector scan,
one can provide a user option indicating which scintillator strip was targeted by the reference detector.
This means that signals from any other strips will be ignored at the lowest possible level,
resulting in much faster processing. For more detailes please read README

TimeWindowCreator_MinTime_float
--- Time Slots have certain duration for specific runs, minimum time is usually
negative.
--- Default value -1*10^6 ps

TimeWindowCreator_MaxTime_float
--- Default value 0.0 ps

SignalFinder_EdgeMaxTime_float
--- time window for matching Signal Channels on Leading Edge.
--- Default value: 5 000 ps.

SignalFinder_LeadTrailMaxTime_float
--- time window for matching Signal Channels on the same thresholds from
Leading and Trailing edge. Default value: 25 000 ps.

HitFinder_VelocityFile_std::string
--- name of ASCII file of required format, containing values of effective velocities
of light in each scintillator.

HitFinder_ABTimeDiff_float
--- time window for matching Signals on the same scintillator and different sides
--- Default value: 6 000 ps.

HitFinder_RefDetScinID_int
--- ID of Reference Detector Scintillator, needed for creating reference hits.
--- The only value  used so far: 193

TimeCalibration_PMIdRef_int
--- Id of the photomultiplier of the refference detector.
--- So far we used always number 385

TimeCalibration_TOTCutLow_float (in the older versions	of Framework: TOTCutLow)
--- Lower bound of TOT for signals used to calibrate slots with refference detector.
We do not use it  so far since the results with cuts were not better then without.
--- Default value: -300000000

TimeCalibration_TOTCutHigh_float (in the older versions of Framework: TOTCutHigh)
---Upper bound of TOT for signals used to calibrate slots with refference detector.
We do not use it  so far since the results with cuts were not better then without.
--- Default value: 300000000.

