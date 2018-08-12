################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Procedures for effective velocity of light calibration.
################################################################################

cmake_minimum_required(VERSION 2.6)

################################################################################
## Project name
set(projectName VelocityCalibration)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  run.sh
  userParams.json
  README.md
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

## Using source files of modules from LargeBarrelAnalysis
file(GLOB HEADERS *.h ../LargeBarrelAnalysis/*.h)
file(GLOB SOURCES *.cpp ../LargeBarrelAnalysis/*.cpp)
file(GLOB LBAE_MAIN_CPP ../LargeBarrelAnalysis/main.cpp)
file(GLOB MAIN_CPP main.cpp)
file(GLOB UNIT_TEST_LBAE_SOURCES ../LargeBarrelAnalysis/*Test.cpp)
file(GLOB ESTVEL_SOURCE estimateVelocity.cpp)
file(GLOB SOURCES_WITHOUT_MAIN *.cpp)
list(REMOVE_ITEM SOURCES ${LBAE_MAIN_CPP})
list(REMOVE_ITEM SOURCES ${ESTVEL_SOURCE})
list(REMOVE_ITEM SOURCES ${UNIT_TEST_LBAE_SOURCES})
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${MAIN_CPP})
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${LBAE_MAIN_CPP})

################################################################################
## Build definitions and libraries linking
include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS} )

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

add_executable(estimateVelocity ${ESTVEL_SOURCE})
target_link_libraries(estimateVelocity JPetFramework)

add_custom_target(clean_data_velocitycalibration
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

set(copy_depends) #create variable for list with depends files path

## Create variable for list with depends files path
foreach(file_i ${AUXILIARY_FILES})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})

  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${file_i})
    set(CP_CMD copy_directory)
  else()
    set(CP_CMD copy)
  endif()
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    )
endforeach( file_i )

## Copy the ROOT scripts
foreach(file_i ${ROOT_SCRIPTS})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    )
endforeach(file_i)

################################################################################
## Unit tests
set(TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})
add_custom_command(OUTPUT ${TESTS_DIR}/unitTestData
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/unitTestData ${TESTS_DIR}/unitTestData
)

## Add new target that depends on copied files
add_custom_target(copy_files_velocity DEPENDS ${copy_depends} ${TESTS_DIR}/unitTestData)

## Add dependencies between building project and copy files
add_dependencies(${projectBinary} copy_files_velocity)
