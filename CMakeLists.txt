################################################################################
## Set of projects based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Script for building J-PET Framework, J-PET MLEM and all the examples
################################################################################

cmake_minimum_required(VERSION 2.6)

################################################################################
## Add Framework submodule
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/j-pet-framework/cmake)
add_subdirectory(j-pet-framework)
include_directories(j-pet-framework ${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

################################################################################
# Add MLEM submodule
add_subdirectory(j-pet-mlem)
#SYSTEM keyword is a trick to disable all warnings from the j-pet-mlem package.
#This code invokes some horrible warnings that we deliberately turn off, to
#keep our build process clean.
include_directories(SYSTEM j-pet-mlem ${MLEM_INCLUDE_DIRS})

################################################################################
add_definitions(-std=c++11 -Wall -Wunused-parameter)

################################################################################
## Enable rpath on OS X and point it to ROOT
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -std=c++11 -Wl,-rpath,${ROOT_LIBRARY_DIR}"
    )
endif()

################################################################################
## Download input and configuration files
## The script shouldn't do anything if the data is present and correct.
set(DOWNLOAD_MORE_DATA ${CMAKE_CURRENT_SOURCE_DIR}/download_reformed_data.sh ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND ${DOWNLOAD_MORE_DATA})

################################################################################
## Directories with examples
add_subdirectory(InterThresholdCalibration)
add_subdirectory(LargeBarrelAnalysis)
add_subdirectory(VelocityCalibration)
add_subdirectory(NewAnalysisTemplate)
add_subdirectory(ImageReconstruction)
add_subdirectory(ScopeLoaderExample)
add_subdirectory(TimeCalibration)
add_subdirectory(MCGeantAnalysis)
add_subdirectory(PhysicAnalysis)
add_subdirectory(CosmicAnalysis)
add_subdirectory(ScopeAnalysis)
add_subdirectory(Imaging)
