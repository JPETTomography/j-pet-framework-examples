################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Image reconstruction for Large Barrel setup
################################################################################
cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

################################################################################
## Project name
set(projectName ImageReconstruction)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  userParams.json
  PARAMETERS.md
  README.md
  run.sh
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/FilterEvents.h
            ${CMAKE_CURRENT_SOURCE_DIR}/ImageReco.h
            ${CMAKE_CURRENT_SOURCE_DIR}/MLEMRunner.h
            ${CMAKE_CURRENT_SOURCE_DIR}/SinogramCreator.h
            ${CMAKE_CURRENT_SOURCE_DIR}/ReconstructionTask.h
            ${CMAKE_CURRENT_SOURCE_DIR}/SinogramCreatorTools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/JPetGojaParser.h
            ${CMAKE_CURRENT_SOURCE_DIR}/../j-pet-mlem/src/util/png_writer.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/FilterEvents.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/ImageReco.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/MLEMRunner.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/SinogramCreator.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/ReconstructionTask.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/SinogramCreatorTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/JPetGojaParser.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/../j-pet-mlem/src/util/png_writer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_subdirectory(Reconstruction)

################################################################################
## Add options from j-pet-mlem as compile flags removing PET_ prefix
## j-pet-mlem sets some parameters of the CMake build by using definitions,
## that are limited to project, it is necessary to copy them here
# configuration time options
set(PET_MAX_DETECTORS 512
    CACHE STRING
    "maximum number of detectors allowed in the scanner (impacts performance)"  )
set(PET_GRANULARITY_TYPE "WARP"
    CACHE STRING
    "warp, thread (or simple just for testing) granularity"  )
set(PET_USE_STATISTICS OFF
    CACHE BOOL
    "collect statistics and print them after program is finished"  )
set(PET_CACHE_ELLIPSE_PIXELS OFF
    CACHE BOOL
    "shared memory pixel buffer in error ellipse"  )
set(PET_USE_SENSITIVITY ON
    CACHE BOOL
    "use sensitivity when calculating reconstruction"  )
set(PET_USE_KERNEL ON
    CACHE BOOL
    "use kernel (when off it uses constant 1 for kernel value)"  )
set(PET_THROW_ON_ZERO_DENOMINATOR OFF
    CACHE BOOL
    "throw on zero denominator (event outside FOV)"  )
set(PET_MAX_PIXELS_PER_THREAD 12
    CACHE STRING
    "chosen arbitrarily, no more than 11 pixels per thread in current config"  )
set(PET_WARP_SIZE 32
    CACHE STRING
    "normally it should not be changed to anything else than 32"  )
set(PET_MAX_THREADS_PER_BLOCK 512
    CACHE STRING
    "not used in current implementation"  )
set(PET_USE_FAST_TEXT_PARSER ON
    CACHE BOOL
    "use fast text parser for loading events (around ~5x faster than C++)"  )
get_cmake_property(vars VARIABLES)
foreach(var ${vars})
  if(var MATCHES "^PET_" AND NOT var MATCHES "_DIR$")
    string(REGEX REPLACE "^PET_" "" name ${var})
    if(name MATCHES "_TYPE$")
      string(REGEX REPLACE "_TYPE$" "" type_name ${name})
      add_definitions(-DUSE_${${var}}_${type_name}=1)
      add_definitions(-D${name}=${${var}})
    elseif(${var})
      if(${var} STREQUAL ON)
        add_definitions(-D${name}=1)
      else()
        add_definitions(-D${name}=${${var}})
      endif()
    else()
      add_definitions(-D${name}=0)
    endif()
  endif()
endforeach()

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework JPetRecoImageTools)
target_include_directories(${projectBinary} PRIVATE
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../j-pet-mlem/src>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../j-pet-mlem/lib/json>
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../j-pet-mlem/lib/cmdline>)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)
################################################################################
## Copy the auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

################################################################################
## Unit tests
option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
