################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Procedures for time calibration.
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

################################################################################
## Project name
set(projectName TimeCalibration_lifetime)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  conf_trb3.xml
  userParams.json
  calibParams.json
  PARAMETERS.md
  README.md
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)

## Project is using only C++
project(${projectName} CXX)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

## Using source files of modules from LargeBarrelAnalysis
set(use_modules_from ../LargeBarrelAnalysis)
set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/CalibrationUnit.h
            ${CMAKE_CURRENT_SOURCE_DIR}/EventFinder.h
            ${use_modules_from}/TimeWindowCreator.h
            ${use_modules_from}/TimeWindowCreatorTools.h
            ${use_modules_from}/UniversalFileLoader.h
            ${use_modules_from}/SignalFinder.h
            ${use_modules_from}/SignalFinderTools.h
            ${use_modules_from}/SignalTransformer.h
            ${use_modules_from}/ToTEnergyConverter.h
            ${use_modules_from}/ToTEnergyConverterFactory.h
            ${use_modules_from}/HitFinder.h
            ${use_modules_from}/HitFinderTools.h
            ${use_modules_from}/EventCategorizerTools.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CalibrationUnit.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/EventFinder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${use_modules_from}/TimeWindowCreator.cpp
            ${use_modules_from}/TimeWindowCreatorTools.cpp
            ${use_modules_from}/UniversalFileLoader.cpp
            ${use_modules_from}/SignalFinder.cpp
            ${use_modules_from}/SignalFinderTools.cpp
            ${use_modules_from}/SignalTransformer.cpp
            ${use_modules_from}/ToTEnergyConverter.cpp
            ${use_modules_from}/ToTEnergyConverterFactory.cpp
            ${use_modules_from}/HitFinder.cpp
            ${use_modules_from}/HitFinderTools.cpp
            ${use_modules_from}/EventCategorizerTools.cpp)

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)

################################################################################
## Copy the auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)