# Example of adding a user-defined custom data class

## Aim
This example demonstrates how to add a custom data class to the set of standard data classes of the J-PET Framework (e.g. JPetPhysSignal, JPetHit, JPetEvent). The custom class used in this example (JPetLORevent) is a special case of an event intended to store 2-gamma annihilation events. It derives from JPetEvent but is extended with additional information about the reconstructed position of the annihilation point on a LOR (stored as a TVector3 from ROOT).

This example can be followed to create other analysis-specific data classes containing additional information used in the analyses.

To demonstrate a real-life use case, this example used the analysis chain from LargeBarrelAnalysis and adds a LORFinder module which identifies 2-gamma events, reconstructs their annihilation points and stores them as JPetLORevent objects in the output file *.lor.evt.root.

## Usage
Run the program the same way to the LargeBarrelAnalysis example.

## Expected result
A ROOT file with the extension *.lor.evt.root will be created. Inspect the file with ROOT's TBrowser to see that JPetTimeWindow objects in the ROOT tree inside contain JPetLORevent objects with a similar structure to JPetEvent objects except that they additionally have a TVector3 field named fAnnihilationPoint containing the reconstructed annihilation point coordinates. The latter can be visualized with:
`root [0] T->Draw("JPetTimeWindow.fEvents.fAnnihilationPoint.fY:JPetTimeWindow.fEvents.fAnnihilationPoint.fX", "", "colz")`

## Details of adding custom classes

 1. Place the .h and .cpp files defining your class in the directory of your program

 2. Edit CMakeLists.txt including the name of your custom class in the `USER_DATA_CLASSES` list 

 3. Include and use your class in your analysis modules just as any of the standard data classes (e.g. as JPetEevnt). CMake will automatically detect your class and build a ROOT dictionary for it. 

## Notes
Please be aware that your class will only be available to the program in whose directory it is defined and not globally in all framework-based programs.

## Author
[Aleksander Gajos](https://github.com/alekgajos)  
Please report any bugs and suggestions of corrections to: [aleksander.gajos@uj.edu.pl](aleksander.gajos@uj.edu.pl)