################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Basic data reconstruction for Large Barrel setup
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()


################################################################################
## Project name
set(projectName LargeBarrelAnalysis)

## Auxiliary files
set(AUXILIARY_FILES
  dummyCalibration.txt
  userParams.json
  conf_trb3.xml
  PARAMETERS.md
  README.md
  run.sh
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizerTools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/EventFinder.h
            ${CMAKE_CURRENT_SOURCE_DIR}/HitFinder.h
            ${CMAKE_CURRENT_SOURCE_DIR}/HitFinderTools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/SignalFinder.h
            ${CMAKE_CURRENT_SOURCE_DIR}/SignalFinderTools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/SignalTransformer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/TimeWindowCreator.h
            ${CMAKE_CURRENT_SOURCE_DIR}/TimeWindowCreatorTools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/UniversalFileLoader.h
            ${CMAKE_CURRENT_SOURCE_DIR}/ToTConverter.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizerTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/EventFinder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/HitFinder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/HitFinderTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/SignalFinder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/SignalFinderTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/SignalTransformer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/TimeWindowCreator.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/TimeWindowCreatorTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/UniversalFileLoader.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/ToTConverter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)


add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

################################################################################
## Copy the example auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

################################################################################
## Unit tests
option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
