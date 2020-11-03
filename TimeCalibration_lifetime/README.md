Aim
---
This is an example analysis producing J-PET time calibration using data taken for a single source inside the detector.

Input Data
-----------
For a single execution of the program, a HLD file should be used, from the measurement.

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


Additional info
--------------

Compiling 
------------
make

Running
------------


Author
------------
Kamil Dulski
Please report any bugs and suggestions of corrections to: <kamil.dulski@gmail.com>
