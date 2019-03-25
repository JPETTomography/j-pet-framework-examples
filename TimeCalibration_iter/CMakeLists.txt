################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Procedures for time calibration.
################################################################################

cmake_minimum_required(VERSION 2.6)

################################################################################
## Project name
set(projectName TimeCalibration_iter)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  conf_trb3.xml
  userParams.json
  PARAMETERS.md
  README.md
  run_calibAll.sh
)

set(ROOT_SCRIPTS
  rootlogon.C
  rootalias.C
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)

## Project is using only C++
project(${projectName} CXX)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

######################################################################
## Using source files of modules from LargeBarrelAnalysis
set(use_modules_from ../LargeBarrelAnalysis)
list(APPEND HEADERS ${use_modules_from}/TimeWindowCreator.h)
list(APPEND SOURCES ${use_modules_from}/TimeWindowCreator.cpp)
list(APPEND HEADERS ${use_modules_from}/TimeWindowCreatorTools.h)
list(APPEND SOURCES ${use_modules_from}/TimeWindowCreatorTools.cpp)
list(APPEND HEADERS ${use_modules_from}/UniversalFileLoader.h)
list(APPEND SOURCES ${use_modules_from}/UniversalFileLoader.cpp)
list(APPEND HEADERS ${use_modules_from}/SignalFinder.h)
list(APPEND SOURCES ${use_modules_from}/SignalFinder.cpp)
list(APPEND HEADERS ${use_modules_from}/SignalFinderTools.h)
list(APPEND SOURCES ${use_modules_from}/SignalFinderTools.cpp)
list(APPEND HEADERS ${use_modules_from}/SignalTransformer.h)
list(APPEND SOURCES ${use_modules_from}/SignalTransformer.cpp)
list(APPEND HEADERS ${use_modules_from}/HitFinder.h)
list(APPEND SOURCES ${use_modules_from}/HitFinder.cpp)
list(APPEND HEADERS ${use_modules_from}/HitFinderTools.h)
list(APPEND SOURCES ${use_modules_from}/HitFinderTools.cpp)

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)

## Create variable for list with depends files path
set(copy_depends)

################################################################################
## Copy the auxiliary files
foreach(file_i ${AUXILIARY_FILES})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${file_i})
    set(CP_CMD copy_directory)
  else()
    set(CP_CMD copy)
  endif()
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
  )
endforeach(file_i)

## Copy the ROOT scripts
foreach(file_i ${ROOT_SCRIPTS})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
  )
endforeach(file_i)

################################################################################
## Add new target that depends on copied files
add_custom_target(copy_files_${projectName} DEPENDS ${copy_depends})
## Add dependencies between building project and copy files
add_dependencies(${projectBinary} copy_files_${projectName})
