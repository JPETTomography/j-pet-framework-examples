# J-PET Analysis Framework Examples

This README file describes the Repository of Usage Examples of the J-PET Analysis Framework.
For details of the Framework refer to the [Framework README](j-pet-framework/README.md)

## What is it?
**J-PET Analysis Framework** is a flexible analysis environment which serves as a backbone system
for the reconstruction algorithms and calibration procedures used during the data processing
and standardizes the common operations, e.g: input/output process, access to the detector
geometry parameters and more. It is written in C++ using the object-oriented approach.
It is based on the ROOT libraries combined with some BOOST packages. The quality of the code
is assured by the automatized set of unit tests. The documentation of the code is generated by Doxygen.

The J-PET Framework Examples Repository is a set of programs built on top of the J-PET Analysis Framework C++ library.
These programs are meant to demonstrate the usage of this library as well as to be starting point for new analysis programs created by users.

The J-PET Framework library is a separate git repository available on [GitHub](http://www.github.com/JPETTomography/j-pet-framework.git)

It is a submodule of this (examples) repository. If you follow the instruction below, the sources of the The J-PET Framework library will be automatically downloaded and placed in a directory named `j-pet-framework`.

Current available programs in this repository:
  * Large Barrel Analysis - starting point modules for reconstruction of Big Barrel data analysis.
  * Time Calibration - procedures for calculating calibration constants from measurements with the reference detector.
  * Velocity Calibration - procedures for calculating effective light velocity in the scintillators.
  * Cosmic, Physics, Imaging - procedures with approach for data streaming.
  * Image Reconstruction - procedures for producing sinograms.
  * Scope loader and analysis - handling scope data.
  * New Analysis Template - project, that can be modified for custom user analysis.

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

Authors of the J-Pet Analysis Framework core are listed in the [README](j-pet-framework/README.md) of the Framework repository.

## Bug Reporting & Contact
If you have any question or comment please write to:
[wojciech.krzemien@ncbj.gov.pl](wojciech.krzemien@ncbj.gov.pl)
or better post it to the [Redmine](http://sphinx.if.uj.edu.pl/redmine/)
