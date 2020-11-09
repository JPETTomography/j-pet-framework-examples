Aim
---
This is an example analysis producing J-PET time calibration using data taken for a single source inside the detector. Calibration is divided into two steps: calibration of a single module based on the time difference AB, and calibration between modules based on the lifetime spectra.

Input Data
-----------
For a single execution of the program, a HLD file should be used, from the measurement. Usually measurement from 30 min or an hour, with the source ~1MBq should be enough to produce proper correction for every scintillator.

Expected output
---------------
No output to stdout.
JPet.log file appears with the log of the processing and ROOT files with the following etensions are produced:
 *.tslot.raw.root
 *.raw.sig.root
 *.phys.sig.root
 *.hits.root
 *.calib.root
where "*" stands for the name of the output file (constructed as LayerId_slotId_FileId, e.g. for the first file measured for first scintillator in the first layer: "Layer1_slot01_1")

Description
--------------
Full time calibration based on the lifetime spectra. 
Requirements - source with the positrons that annihilate near the source.

Both sides of each scintillator are calibrated based on the time difference AB distribution. For calibrated scintillator time difference AB should be centered at zero.
Position of the time difference AB distribution is estimated from the edges. Middle of the edge is estimated from the second derivative from the time difference AB distribution. Correction is calculated as MiddleEdge(Left Edge) - MiddleEdge(Right Edge).
Correction is applied for a B side only.

Calibration between modules is done iteratively. After AB calibration events with only one photon coming from positron-electron annihilation and only one photon coming from the deexcitation of the source is selected based on the TOT values. For every event lifetme estimator is calculated as Time(AnnihilationHit) - Time(DeexcitationHit). Positron lifetime distribution is filled for two histograms - for a given scintillator ID with annihilation hit and a given scintillator ID with deexcitation hit. Correction for a given scintillator is a difference between maximum of the distribution when annihilation photon hit a given ID and maximum of the distribution when deexcitation photon hit a given ID.

At the end calibration file is created according to the convention.

Additional info
--------------

Compiling 
------------
make

Running
------------
standard running as for the LargeBarrelAnalysis.
First step of calibration:
- running analysis till EventFinder
- hadding unk.evt.root files
- compiling Calibrate module
- running ./Calibrate.exe [file with calibration params (calibParams.json)] single

Second step of calibration:
- running analysis for CalibrationUnit
- hadding cat.evt.root files
- running ./Calibrate.exe [file with calibration params (calibParams.json)] multi

Third step of analysis:
- repeating the second step until corrections are lower than uncertainities
- running ./Calibrate.exe [file with calibration params (calibParams.json)] final

Author
------------
Kamil Dulski
Please report any bugs and suggestions of corrections to: <kamil.dulski@gmail.com>
