################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2022
##
## Description:
##   Experimental analysis with first prototype of J-PEM, aka Red Module
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

######################################################################
## Project name RedModuleAnalysis
######################################################################
set(projectName RedModuleAnalysis)

## Auxiliary files are copied to build directory after CMake
## You can add any files to this list
set(AUXILLIARY_FILES
  README.md
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX) # using only C++

## Using source files of modules from ModularDetectorAnalysis
set(use_modules_from ../ModularDetectorAnalysis)
######################################################################
## Add your own HEADERS/SOURCES
set(HEADERS ${use_modules_from}/TimeWindowCreator.h
            ${use_modules_from}/TimeWindowCreatorTools.h
            ${use_modules_from}/SignalFinder.h
            ${use_modules_from}/SignalFinderTools.h
            ${use_modules_from}/SignalTransformer.h
            ${use_modules_from}/SignalTransformerTools.h
            RedModuleHitFinder.h
            RedModuleHitFinderTools.h
            ${use_modules_from}/EventFinder.h
            #${use_modules_from}/EventCategorizer.h
            #${use_modules_from}/EventCategorizerTools.h
            )

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${use_modules_from}/TimeWindowCreator.cpp
            ${use_modules_from}/TimeWindowCreatorTools.cpp
            ${use_modules_from}/SignalFinder.cpp
            ${use_modules_from}/SignalFinderTools.cpp
            ${use_modules_from}/SignalTransformer.cpp
            ${use_modules_from}/SignalTransformerTools.cpp
            RedModuleHitFinder.cpp
            RedModuleHitFinderTools.cpp
            ${use_modules_from}/EventFinder.cpp
            #${use_modules_from}/EventCategorizer.cpp
            #${use_modules_from}/EventCategorizerTools.cpp
            )

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)

################################################################################
## Copy the example auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
