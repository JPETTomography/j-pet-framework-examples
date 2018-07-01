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

# add options from j-pet-mlem as compile flags removing PET_ prefix
get_cmake_property(vars VARIABLES)
foreach(var ${vars})
  if(var MATCHES "^PET_" AND NOT var MATCHES "_DIR$")
    string(REGEX REPLACE "^PET_" "" name ${var})
    if(name MATCHES "_TYPE$")
      string(REGEX REPLACE "_TYPE$" "" type_name ${name})
      add_definitions(-DUSE_${${var}}_${type_name}=1)
      add_definitions(-D${name}=${${var}})
      message(STATUS "Using ${${var}} ${type_name}")
    elseif(${var})
      if(${var} STREQUAL ON)
        add_definitions(-D${name}=1)
        message(STATUS "Enabled ${name}")
      else()
        message(STATUS "Using ${name}=${${var}}")
        add_definitions(-D${name}=${${var}})
      endif()
    else()
      add_definitions(-D${name}=0)
      message(STATUS "Disabled ${name}")
    endif()
  endif()
endforeach()

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

