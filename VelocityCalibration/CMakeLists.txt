################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Procedures for effective velocity of light calibration.
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

################################################################################
## Project name
set(projectName VelocityCalibration)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  conf_trb3.xml
  userParams.json
  README.md
  run.sh
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)

## Project is using only C++
project(${projectName} CXX)

## Using source files of modules from LargeBarrelAnalysis
set(use_modules_from ../LargeBarrelAnalysis)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/DeltaTFinder.h
            ${use_modules_from}/TimeWindowCreator.h
            ${use_modules_from}/TimeWindowCreatorTools.h
            ${use_modules_from}/UniversalFileLoader.h
            ${use_modules_from}/SignalFinder.h
            ${use_modules_from}/SignalFinderTools.h
            ${use_modules_from}/SignalTransformer.h
            ${use_modules_from}/HitFinder.h
            ${use_modules_from}/HitFinderTools.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/DeltaTFinder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${use_modules_from}/TimeWindowCreator.cpp
            ${use_modules_from}/TimeWindowCreatorTools.cpp
            ${use_modules_from}/UniversalFileLoader.cpp
            ${use_modules_from}/SignalFinder.cpp
            ${use_modules_from}/SignalFinderTools.cpp
            ${use_modules_from}/SignalTransformer.cpp
            ${use_modules_from}/HitFinder.cpp
            ${use_modules_from}/HitFinderTools.cpp)

set(ESTVEL_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/estimateVelocity.cpp)


add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

add_executable(estimateVelocity ${ESTVEL_SOURCE})
target_link_libraries(estimateVelocity JPetFramework::JPetFramework)

add_custom_target(clean_data_velocitycalibration
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)

## Create variable for list with depends files path
set(copy_depends)

################################################################################
## Create variable for list with depends files path
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


################################################################################
## Add new target that depends on copied files
add_custom_target(copy_files_velocity DEPENDS ${copy_depends})
## Add dependencies between building project and copy files
add_dependencies(${projectBinary} copy_files_velocity)
