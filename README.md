# J-PET Analysis Framework Examples

This README file describes the Repository of Usage Examples of the J-PET Analysis Framework.
For details of the **J-PET Analysis Framework** refer to the [this README](https://github.com/JPETTomography/j-pet-framework/blob/master/README.md)  

The **J-PET Framework** is a separate git repository available on [GitHub](http://www.github.com/JPETTomography/j-pet-framework.git)

## What is it?
Usage Examples utilize Framework library to provide backbone for data processing. The programs contained in this repository are meant to demonstrate procedures for calibrations, reconstruction and anaysis of data from J-PET experiment as well as providing starting point for new programs and set of tools for users.  

Short description of programs in this repository:  
  * Large Barrel Analysis - basic reconstruction of Big Barrel data, from the low-level binary to higher level objects representing physicial phenomena, registered by the detector;  
  * Time Calibration - procedures for calculating calibration constants from measurements with the reference detector. Second calibration procedure introduces iterative approach of data calibration;  
  * Velocity Calibration - procedures for calculating effective light velocity in the scintillators;  
  * Inter threshold calobration - procedures for calculating time-walk effect;  
  * Cosmic, Physics, Imaging - procedures with approach for data streaming;  
  * Image Reconstruction - procedures for producing sinograms and reconstructing image. It is dependant on [j-pet-mlem](https://github.com/JPETTomography/j-pet-mlem) project;  
  * Scope loader and analysis - procedures handling scope data;  
  * MCGeantAnalysis - conversion of results of Monte Carlo simulations prepared with the J-PET simulations package [J-PET-Geant4](https://github.com/JPETTomography/J-PET-geant4) to a format consistent with the Framework data sturctures;  
  * New Analysis Template - project, that can be modified for custom user analysis;  
  * User Data Class - example of adding own class to Framework, to be utilized in custom physics analisis.  

## The Latest Version
The latest stable version can be downloaded from the github repository. You must have git client installed and do:

`git clone --recursive https://github.com/JPETTomography/j-pet-framework-examples.git`

If you have already a directory with the code and you want to update it, just write:

`git pull && git submodule update`

## Installation
Please see the file called [INSTALL](INSTALL.md).

## Authors
J-PET Analysis Framework Examples are being maintained by [Aleksander Gajos](https://github.com/alekgajos), [Wojciech Krzemien](https://github.com/wkrzemien) and [Kamil Rakoczy](https://github.com/grey277).
Contributors: [Szymon Niedźwiecki](https://github.com/wictus), Oleksandr Rundel, Tomasz Kisielewski, Nikodem Krawczyk, Klara Muzalewska, Monika Pawlik-Niedźwiecka, Kamil Dulski, Michał Silarski, Magdalena Skurzok, [Krzysztof Kacprzak](https://github.com/kkacprzak), Elena Pérez del Río.

Authors of the J-Pet Analysis Framework core are listed in the [README](https://github.com/JPETTomography/j-pet-framework/blob/master/README.md) of the Framework repository.

## Bug Reporting & Contact
If you have any question or comment please write to:
[wojciech.krzemien@ncbj.gov.pl](wojciech.krzemien@ncbj.gov.pl)
or better post it to the [Redmine](http://sphinx.if.uj.edu.pl/redmine/)
