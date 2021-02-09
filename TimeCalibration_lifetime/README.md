Aim
---
This is an example analysis producing J-PET time calibration using data taken for a single source inside the detector. Calibration is divided into two steps: calibration of a single module based on the time difference AB, and calibration between modules based on the lifetime spectra.
At the end it can be also used to estimate optimal effective length of a scintillator onto different thresholds.

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
 *.unk.evt.root
 *.cal.it.root
where "*" stands for the name of file

Description
--------------
Full time calibration based on the lifetime spectra. 
Requirements - source with the positrons that annihilate near the source.

Both sides of each scintillator are calibrated based on the time difference AB distribution. For calibrated scintillator time difference AB should be centered at zero.
Position of the time difference AB distribution is estimated from the edges. Middle of the edge is estimated from the second derivative from the time difference AB distribution. Correction is calculated as MiddleEdge(Left Edge) - MiddleEdge(Right Edge).
Correction is applied for a B side only.

Calibration between modules is done iteratively. After AB calibration events with only one photon coming from positron-electron annihilation and only one photon coming from the deexcitation of the source is selected based on the TOT values. For every event lifetme estimator is calculated as Time(AnnihilationHit) - Time(DeexcitationHit). Positron lifetime distribution is filled for two histograms - for a given scintillator ID with annihilation hit and a given scintillator ID with deexcitation hit. Correction for a given scintillator is a difference between maximum of the distribution when annihilation photon hit a given ID and maximum of the distribution when deexcitation photon hit a given ID.

At the end calibration file is created according to the convention.

When calibration is done for different effective lengths, an optimal value of the effective length can be estimated using EstimateEffectiveLength.exe.  

Additional info
--------------
Report is available on petwiki
petwiki/images/2/2f/Calibration_Report_11_12.pdf

Compiling 
------------
make

Running
------------
standard running as for the LargeBarrelAnalysis.
First step of calibration:
- running analysis till EventFinder
- hadding unk.evt.root files
- updating calibration params file with the name of the hadded files and other options
- running ./Calibrate.exe [file with calibration params (calibParams.json)] single

Second step of calibration:
- running analysis for PALSCalibrationTaks
- hadding cal.it.root files
- updating calibration params file with the name of the hadded files and other options
- running ./Calibrate.exe [file with calibration params (calibParams.json)] multi

Third step of analysis:
- repeating the second step until corrections are lower than uncertainities
- running ./Calibrate.exe [file with calibration params (calibParams.json)] final

Additional step of analysis
- performing calibration for different values of the effective length paramater in a calibration params file
- updating effective length params file (default effLenParams.json) with the names of the files with final correction for different effective lengths
- running ./EstimateEffectiLength.exe [file with effective length params (effLenParams.json)]

Author
------------
Kamil Dulski
Please report any bugs and suggestions of corrections to: <kamil.dulski@gmail.com>
