# Calibration scripts for Modular Detector data

## Scripts

  This folder contains ROOT macros, that calculate calibration constants and produce a JSON file in the specific format.  

1. `channel_offests.C` - initial synchronization of channels in SiPM of the same matrix, using `mtx_channel_offsets` histogram from `SignalTransformer` task  

2. `matrix_offsets.C` - similar channel synchronization, but uses `evtcat_channel_offsets` histogram from `EventCategorizer` with annihilation events, corrects coarse offsets from previous script  

3. `hit_tdiff_effvel.C` - calculating B side signal time correction and effective velocity of light for each scintillators, based on `hit_tdiff_scin` histogram  

4. `tot_norm_sipm.C` and `tot_norm_scin.C` - scripts for calculating ToT normalization constants for spectra for SiPM signals or Hits, histograms `tot_sipm_id` from `SignalFinder` and `hit_tot_scin` from `HitFinder` respectively. The working principal is the same, some hard-coded constants have to be adjusted for histogram limits or Compton edges placement.  

5. `tof_synchro.C` - synchronization of hit registration time in all the strips in the detector. Reads two histograms from `EventCategorizer`: `tdiff_anni_scin` and `tdiff_deex_scin`. This procedure should be performed many times, which can be automated with bash script `run_tof_synchro.sh`.  

6. `time_walks.C` - calculating time walk effect correction based on the histogram `time_walk_ab_tdiff` from `EventCategorizer`. The limits of the linear fit can be adjusted based on values of reversed ToT histogram.  

## Preparation of input files  

  Calibration procedures need to be performed on data from Modular J-PET measurement with the source in the center. The example analysis `ModularDetectorAnalysis` should be executed with user options `Save_Control_Histograms_bool` and `Save_Calib_Histograms_bool` set to `true` in order to fill additional histograms. Then the histograms can be extracted from output files with `rootcp` command:  

  `rootcp dabc_20296162858.cat.evt.root:*/* cat.root`  

  Or for multiple files of the same type can be merged using `hadd` tool from ROOT package, providing `-T` option for copying histograms (not trees with data)  
  `hadd -T histos.root *.cat.evt.root`  

## How to run  

  All scripts have a similar way of running, for that reason only one example is described in detail and others are provided with general definition. Start ROOT console in batch mode (to avoid showing multitude of canvases on-screen):  
  `root -l -b`  

* Load macro:  
  `.L matrix_offsets.C`  

* Run macro providing input file and all the default options:  
  `matrix_offsets("mtx.root")`  
  this uses `mtx.root` file, prepared as described in the section above, and produces a file with calibration constants `calibration_constants.json` in the current directory.  

* Specify JSON file name and directory:  
  `matrix_offsets("mtx.root", "/home/user123/results/my_calibration_1.json")`  

* Save canvases as `PNG` files, that are created during the run, also a output directory can be specified (if not, current directory is used):
  `matrix_offsets("mtx.root", "my_calibration_1.json", true, "results/histo_png/")`  

* Specify minimal and maximal ID number of objects to be iterated (ID of photomultipliers or scintillators).  
  `matrix_offsets("mtx.root", "my_calibration_1.json", flase, "./", 513, 742)`  

* The scipr `tof_synchro.C` used an additional variable (optional), specifying iteration (default =0):
  `void tof_synchro(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false, std::string resultDir = "./", int iteration = 0, int minScinID = 201, int maxScinID = 512)`

## Output JSON file with calibration constants  

  In case if calibration file does not exist, scripts produce a new file (with default name `calibration_constants.json`, if an optional string was not provided). If exists, results of calculations are added to the file or are overwriting existing information of the same type (other fields remain unchanged). Such a file can be used in the example `ModularDetectorAnalysis` by adding the option `ConstantsFile_std::string` in the user options file.
