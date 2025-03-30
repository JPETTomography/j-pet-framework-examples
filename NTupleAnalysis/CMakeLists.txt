################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2025
##
## Description:
##   Exporting files form Event Categorizer to a reduced TTree
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

######################################################################
## Project name - set this section appropriately for your project
## (please remove this comment in your template-derived analysis)
######################################################################
set(projectName NTupleAnalysis)

## Auxiliary files are copied to build directory after CMake
## You can add any files to this list
set(AUXILLIARY_FILES
  README.md
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX) # using only C++

######################################################################
## If you are using your own version of some module which is also present
## in LargeBarrelAnalysis, comment out the respective lines below
## so as not to use this module's source from LargeBarrelAnalysis.
## (please remove this comment in your template-derived analysis)
## Add your own HEADERS/SOURCES
set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/ReadNTuple.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/ReadNTuple.cpp)

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)

################################################################################
## Copy the example auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
