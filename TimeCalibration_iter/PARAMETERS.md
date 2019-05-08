Description of available parameters in TimeCalibration example.
Note that configuration/calibration filed can be obtained from PetWiki:
http://koza.if.uj.edu.pl/petwiki/index.php/Default_settings_and_parameters_used_in_the_analyses
--------

Save_Cotrol_Histograms
--- Common for each module, if set to true, in the output ROOT files folder with
statistics will contain control histograms. Set to false if histograms not needed.

StopIteration_bool
---A flag indicating, that the module is called in the iterative mode with number of iterations defined in
main.cpp as the last argument (true) or with user-defined condition to exit the loop (false). In the latter case
one needs to put -1 as the last argument of manager.useTask (see the description of the TimeCalibration_NiterMax_int)
---Default value: true

Unpacker_TOToffsetCalib_std::string  
---Path to and name of a `ROOT` file with `TOT` offset calibrations (stretcher) applied during unpacking of `HLD` file.

Unpacker_TDCnonlinearityCalib_std::string  
---Path to and name of a `ROOT` file with calibrations of nonlinearities occurring on `TDC` boards, applied during unpacking of `HLD` file.

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

SignalFinder_UseCorruptedSigCh_bool
---Indication if Signal Finder module should use signal channels flagged as Corrupted in the previous task.
Default value: false

HitFinder_VelocityFile_std::string
--- name of ASCII file of required format, containing values of effective velocities
of light in each scintillator.

HitFinder_ABTimeDiff_float
--- time window for matching Signals on the same scintillator and different sides
--- Default value: 6 000 ps.

HitFinder_RefDetScinID_int
--- ID of Reference Detector Scintillator, needed for creating reference hits.
--- The only value  used so far: 193

HitFinder_UseCorruptedSignals_bool  
--- Indication if Hit Finder module should use signals flagged as Corrupted in the previous task.
--- Default value: `false`

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

TimeCalibration_LoadConstants_bool
---The flag indicating if the temporary calibration constants should be loaded after each iteration of the calibration.
Use it only if You disable the calibration loading module (by default it is off), by default it is set to "true"
since this is much faster than the full analysis done starting from unpacker stage. If You really want to use corrections
at that level first uncomment in main.cpp the CalibLoader module and change TimeCalibration_LoadConstants_bool to false.
--- Default value: true

TimeCalibration_OutputFileTmp_std::string
---The file which is used to store the temporary calibration constants after each iteration.
--- Default value: TimeConstantsCalibTmp.txt

TimeCalibration_OutputFileFinal_std::string
---The final output file with calibration constants
--- Default value: TimeConstantsCalib.txt

TimeCalibration_NiterMax_int
--- The user defined number of iterations of calibration. Use it with option "StopIteration_bool" set to false and with
manager.useTask("TimeCalibration", "hits", "calib",-1) in the main.cpp
--- Default value: 1
