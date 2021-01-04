# Calibration scripts for Modular Detector data

## Scripts

This folder contains ROOT macros, that calculate calibration constants and produce a JSON file
with the specific format.  

1. `matrix_offsets.C` - description  

2. `hit_tdiff_effvel.C` - description  

3. `tof_synchro.C` - description  

## Preparation of input files

Calibration procedures need to be performed on data from Modular J-PET measurement with the source in the center. The example analysis `ModularDetectorAnalysis` should be executed with user options `Save_Control_Histograms_bool` and  `Save_Calib_Histograms_bool` set to `true` in order to fill additional histograms. Then the histograms can be extracted from output files:

* From `SignalTransformer` files:  
`rootcp dabc_20296162858.mtx.sig.root:Sig*/* mtx1.root`  
* From `HitFinder` files:  
`rootcp dabc_20296162858.hits.root:Hit*/* hit1.root`  
* From `EventCategorizer` files:  
`rootcp dabc_20296162858.cat.evt.root:Event*/* evt1.root`  

Then multiple files of the same type can be merged using `hadd` tool from ROOT package:  
`hadd mtx.root mtx1.root mtx2.root mtx3.root`  
`hadd hit.root hit1.root hit2.root hit3.root`  
`hadd evt.root evt1.root evt2.root evt3.root`  

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

* General definitions  
  `void matrix_offsets(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false, std::string resultDir = "./", int minPMID = 401, int maxPMID = 2896)`  
  `void hit_tdiff_effvel(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false, std::string resultDir = "./", int minScinID = 201, int maxScinID = 512)`  
  `void tof_synchro(std::string fileName, std::string calibJSONFileName = "calibration_constants.json", bool saveResult = false, std::string resultDir = "./", int minScinID = 201, int maxScinID = 512)`  

## Output JSON file with calibration constants
In case if calibration file does not exist, scripts produce a new file (with default name `calibration_constants.json`, if an optional string was not provided). If exists, results of calculations are added to the file or are overwriting existing information of the same type (other fileds remain unchanged). Such a file can be used in the example `ModularDetectorAnalysis` by adding the option `ConstantsFile_std::string` in user options file. 



