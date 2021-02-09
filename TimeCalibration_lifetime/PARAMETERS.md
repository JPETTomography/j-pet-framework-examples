Description of available parameters in TimeCalibration example.
Note that configuration/calibration filed can be obtained from PetWiki:
http://koza.if.uj.edu.pl/petwiki/index.php/Default_settings_and_parameters_used_in_the_analyses
--------

-------------------------userParams.json-------------------------

Save_Control_Histograms_bool
--- Common for each module, if set to true, in the output ROOT files folder with
statistics will contain control histograms. Set to false if histograms not needed.

StopIteration_bool
--- A flag indicating, that the module is called in the iterative mode with number of iterations defined in
main.cpp as the last argument (true) or with user-defined condition to exit the loop (false). In the latter case
one needs to put -1 as the last argument of manager.useTask (see the description of the TimeCalibration_NiterMax_int)
--- Default value: true

Unpacker_TOToffsetCalib_std::string  
--- Path to and name of a `ROOT` file with `TOT` offset calibrations (stretcher) applied during unpacking of `HLD` file.

Unpacker_TDCnonlinearityCalib_std::string  
--- Path to and name of a `ROOT` file with calibrations of nonlinearities occurring on `TDC` boards, applied during unpacking of `HLD` file.

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
--- Indication if Signal Finder module should use signal channels flagged as Corrupted in the previous task.
Default value: false

HitFinder_VelocityFile_std::string
--- name of ASCII file of required format, containing values of effective velocities
of light in each scintillator.

HitFinder_ABTimeDiff_float
--- time window for matching Signals on the same scintillator and different sides
--- Default value: 6 000 ps.

HitFinder_RefDetScinID_int
--- ID of Reference Detector Scintillator, needed for creating reference hits.
--- The only value  used so far: -1

HitFinder_UseCorruptedSignals_bool
--- Indication if Hit Finder module should use signals flagged as Corrupted in the previous task.
--- Default value: `false`

PALSCalibrationTask_Anni_TOT_Cut_Min_float
--- minimal TOT for the hit that come from the annihilation of the positron-electron
--- Default value: 23 000 ps

PALSCalibrationTask_Anni_TOT_Cut_Max_float
--- maximal TOT for the hit that come from the annihilation of the positron-electron
--- Default value: 30 000 ps

PALSCalibrationTask_Deex_TOT_Cut_Min_float
--- minimal TOT for the hit that come from the deexcitation of the source
--- Default value: 35 000 ps

PALSCalibrationTask_Deex_TOT_Cut_Max_float
--- maximal TOT for the hit that come from the deexcitation of the source
--- Default value: 35 000 ps

PALSCalibrationTask_ZpositionCut_float
--- maximal Z position accepted for a hit
--- Default value: 23 cm

PALSCalibrationTask_EffectiveLength_float
--- effective length assumed for a first threshold
--- Default value: 48 cm

PALSCalibrationTask_SourcePosition_TVector3
--- Assumed position of the source
--- Default value: [0.0, 0.0, 0.0], cm3

PALSCalibrationTask_AB_Corrections
--- File with the time correction spearately for every scintillator. If there is none, no corrections will be applied
--- Default value: AB_Corrections.dat

PALSCalibrationTask_PALS_Corrections
--- File with the time correction between scintillators. If there is none, no corrections will be applied
--- Default value: PALS_Corrections.dat

-------------------------calibParams.json-------------------------

File_with_histos_single_std::string
--- Path to and name of a `ROOT` file that contains histograms after EventFinder. It is for calibration of every detection module
separately in a side AB manner (single option)

File_with_histos_multi_std::string
--- Path to and name of a `ROOT` file that contains histograms after PALSCalibrationTask. It is for calibration betwen detection modules
by calibrating PALS spectra (multi option)

File_with_histos_from calibration_single_std::string
--- Name of a `ROOT` file that will contain control spectra from calibration of a single modules separately (single option)

File_with_histos_from calibration_multi_std::string
--- Name of a `ROOT` file that will contain control spectra from calibration between modules at once (multi option)

File_with_constants_single_std::string
--- Name of a `txt` file that will contain AB corrections for every scintillator that can be passed to a PALSCalibrationTask

File_with_constants_single_for_velocity_std::string
--- Name of a `txt` file that will contain velocities for every scintillator that can be passed to a PALSCalibrationTask for
calculations of the Z position and scanning effective lengths

File_with_constants_multi_std::string
--- Name of a `txt` file that will contain scintillator time corrections for every scintillator that can be passed to a PALSCalibrationTask
for the next iteration

File_with_constants_final_std::string
--- Name of a `txt` file that will contain full time correction (AB and from PALS) after the calibration.

File_with_old_constants_final_std::string
--- Path to and name of a `txt` file that contains time calibration filed used before calibration. The correction from this would be used to
produce final file after calibration.

Histo_name_pattern_single_std::string
--- Pattern of a histogram names that will be generated in the EventFinder task, used for the calibration of a singe detection module
--- Default value: TDiff_AB_vs_ID_thr


Annihilation_histo_name_pattern_multi_std::string
--- Pattern of a histogram names that will be generated in the PALSCalibrationTask task, used for the calibration between detection modules
This is the first pattern used for the Annihilation ID vs PALS histogram
--- Default value: Corrected_PALS_vs_AnnihilationID_thr

Deexcitation_histo_name_pattern_multi_std::string
--- Pattern of a histogram names that will be generated in the PALSCalibrationTask task, used for the calibration between detection modules
This is the second pattern used for the Deexcitation ID vs PALS histogram
--- Corrected_PALS_vs_DeexcitationID_thr

Save_derivatives_bool
--- Option to save derivatives that are calculated during time calibration. They will be saved in a proper file indicated by
File_with_histos_from calibration_single(multi)_std::string option.
--- Default value: true

Number_of_thresholds_int
--- Number of threshold for which the hitograms will be generated
--- Default value: 4

Minimal_Scintillator_ID_int
--- Id of the first detection module that calibration should start. There is option not to calibrate some of the modules
--- Default value: 1

Maximal_Scintillator_ID_int
--- Id of the last detection module that calibration should start. There is option not to calibrate some of the modules
--- Default value: 192

Number_of_points_to_filter_int
--- Number of points that linear filter should work on. Filter is used to smooth derivatives and to estimate corrections better
--- Default value: 6

Threshold_for_derivative_int
--- Value used for finding the first estimation of the extremum on the derivatives. If it is too high, Calibrate.exe would
show for many modules that the threshold is too high. If it is too low, extremum estimates will be wrongly estimated.
--- Default value: 6

Effective_Length_float
--- Value of the effective length that are used to estimate velocity of every module. It can be changed and different PALS
corrections can be calculated. So, the effective length scan can be done from the EventFinder taks, speeding up the whole process

-------------------------effLenParams.json-------------------------

Number_of_files_int
--- Number of files that are used for the effective length scan. Every file should ocrrespond to different effective length value
for which calibration was done. It is used to scan the effLenParams.json file in order to find every file and length. 
If the number will be equal to 3, EstimateEffectiveLength.exe will look for options:
File_1_std::string
File_2_std::string
File_3_std::string
Length_1_float
Length_2_float
Length_3_float
if it will be equal to 2:
File_1_std::string
File_2_std::string
Length_1_float
Length_2_float
so the number of options are fluent and can change with different number of files

File_%d_std::string
--- Path to and name of a `txt` file that contains corrections for a given effective length indicated by 
Length_%d_float option. %d should be equal to the number of a given file, for example: File_1_std::string

Length_%d_float
--- value of a  effective length that was used during calibration. %d should be equal to the number of a given file
for example: Length_1_float

Reference_file_ID_int
--- number of a %d in above options that should be used as a reference file, from which mean correction difference
will be calculated. It should no change the results with different number, but it should be lower than the maximal %d
for which File_%d_std::string and Length_%d_float is given

Output_root_file_std::string
--- Name of a `ROOT` file that will contain results from the estimation of the effective length.
