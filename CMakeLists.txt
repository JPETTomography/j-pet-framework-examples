################################################################################
## J-PET tools and examples
##
## Author: Adam Strzelecki <adam.strzelecki@uj.edu.pl>
##
## Description:
##   Builds J-PET tools and examples depending on Framework.
################################################################################

cmake_minimum_required(VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/j-pet-framework/cmake)

################################################################################
## Add Framework submodule
add_subdirectory(j-pet-framework)
include_directories(j-pet-framework ${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_definitions(-std=c++11 -Wall -Wunused-parameter)

################################################################################
# Add MLEM submodule
add_subdirectory(j-pet-mlem)
include_directories(j-pet-mlem ${MLEM_INCLUDE_DIRS})

################################################################################
## Enable rpath on OS X and point it to ROOT
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -std=c++11 -Wl,-rpath,${ROOT_LIBRARY_DIR}"
    )
endif()

################################################################################
## Download input and config files
## The script shouldn't do anything if the data is present and correct.
set(DOWNLOAD_MORE_DATA ${CMAKE_CURRENT_SOURCE_DIR}/download_reformed_data.sh ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND ${DOWNLOAD_MORE_DATA})

################################################################################
## Directories with examples
add_subdirectory(LargeBarrelAnalysis)
add_subdirectory(VelocityCalibration)
add_subdirectory(NewAnalysisTemplate)
add_subdirectory(ScopeLoaderExample)
add_subdirectory(TimeCalibration)
add_subdirectory(ScopeAnalysis)
add_subdirectory(modules)
add_subdirectory(ImageReconstruction)
add_subdirectory(PhysicAnalysis)
add_subdirectory(CosmicAnalysis)
add_subdirectory(Imaging)




