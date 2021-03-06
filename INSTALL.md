# Installation

## Basic steps

1. Install the required libraries and tools.  
   Refer to the `Requirements` section of this document for details.  
   You can find ready commands for some systems in the `Requirements installation` section.

2. Create a directory where the built programs will be placed, e.g. in the main directory:  
   `mkdir build`  
   `cd build`

3. Before calling cmake, make sure that correct paths to JPetFramework are set with:
   `source path/to/framework/bin/thisframework.sh` 

3. Invoke the CMake build tool and provide the path to the main directory
   If you created the build directory in the main one and entered `build` as in step 2. then do:  
   `cmake ..`

4. To compile the J-PET Framework Library do:  
   `make`

5. Once the compilation is finished, there should be a file `libJPetFramework.so` in the build directory.

**NOTE:** Full install procedure with tips and troubleshootung can be found on [PetWiki](http://koza.if.uj.edu.pl/petwiki/index.php/Installing_the_J-PET_Framework_on_Ubuntu)


## Code documentation

To generate the code documentation of the J-PET Framework library, the Doxygen software must be installed.
To generate the documentation just go to the j-pet-framework build directory and do:
```
make documentation
```
and the documentation will be generated and put in folders named latex and html inside the build directory.


## Requirements
1. gcc

2. [cmake](https://cmake.org/)

3. [ROOT](http://root.cern.ch)  
   Works with ROOT 5 (recommended 5.34/26)
   and ROOT 6 (tested 6.10/08)

4. [BOOST](https://www.boost.org/)

5. [FFTW3](http://www.fftw.org/)

Optional, if you want to generate documentation: [Doxygen](www.doxygen.org)
