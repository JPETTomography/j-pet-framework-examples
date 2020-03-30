# Template for custom analysis

## Aim
This is an example of creating one's own analysis based on the `LargeBarrelAnalysis`. You can start your own analysis by copying the `NewAnalysisTemplate` directory.

## Starting you own Analysis
When starting you own analysis, do not edit any of the existing examples as this will lead to problems with Git-based version control. Instead, follow this procedure:

 1. Copy the `NewAnalysisTemplate` directory to a name you wish to give to you analysis. Let us use `MyAnalysis` as an example:  
    `cp -r NewAnalysisTemplate MyAnalysis`

 2. Add the following line at the end of the `j-pet-framework-examples/CMakeLists.txt`:  
    `add_subdirectory(MyAnalysis)`  
    (replace `MyAnalysis` with whichever name you gave to your directory)

 3. Edit the file `CMakeLists.txt` located in you new directory, e.g. `MyAnalysis/CMakeLists.txt` and and in the following line:  
    `set(projectName NewProjectName)`  
    replace `NewProjectName` with your project name, e.g. `MyAnalysis`

 4. Furter in the `CMakeLists.txt` file, you will see lines responsible for using modules from the `LargeBarrelAnalysis` example. In case you want to customize any of the modules, copy this modules `.h` and `.cpp` files to your directory and comment out the lines concerning this module.

 5. Now when you follow the standard procedure to configure and build all of the J-PET framework examples, a directory with the name you chose will be created in your build directory, e.g. `build/MyAnalysis` and will contain an executable with the name you chose for the project, e.g. `MyAnalysis.x`.

## Additional Info
Note that the `TimeCalibration` and `VelocityCalibration` examples follow exactly the above scheme of being based on `LargeBarrelAnalysis` and its modules, so you can refer to these examples for advanced usage of the template described here.

## Author
[Aleksander Gajos](https://github.com/alekgajos)  
Please report any bugs and suggestions of corrections to: [aleksander.gajos@uj.edu.pl](aleksander.gajos@uj.edu.pl)