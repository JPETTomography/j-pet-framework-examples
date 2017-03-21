# J-PET tools and examples
#
# Author: Adam Strzelecki <adam.strzelecki@uj.edu.pl>
#
# Description:
#   Builds J-PET tools and examples depending on Framework.

cmake_minimum_required(VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/j-pet-framework/cmake)

# add Framework submodule
add_subdirectory(j-pet-framework)
include_directories(j-pet-framework ${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_definitions(-std=c++11 -Wall -Wunused-parameter)

# enable rpath on OS X and point it to ROOT
if(APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -std=c++11 -Wl,-rpath,${ROOT_LIBRARY_DIR}"
    )
endif()

##download input files and config files
#only once
set(DOWNLOAD_DATA ${CMAKE_CURRENT_SOURCE_DIR}/download_data.sh ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT DOWNLOAD_EXAMPLE_DATA_HAPPENED)
  execute_process( COMMAND ${DOWNLOAD_DATA} )
  set(DOWNLOAD_EXAMPLE_DATA_HAPPENED TRUE CACHE BOOL "Has the download data happened?" FORCE)
endif() 
add_subdirectory(AnalysisExample)
add_subdirectory(ScopeLoaderExample)
add_subdirectory(ScopeAnalysis)
add_subdirectory(LargeBarrelAnalysis)
add_subdirectory(LargeBarrelAnalysisExtended)

