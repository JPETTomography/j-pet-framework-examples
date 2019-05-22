################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Analysis of the scope data.
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

################################################################################
## Project name
set(projectName ScopeAnalysis)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  README.md
  run.sh
)

################################################################################
## Binary, header and source files and additional sources definitions
set(projectBinary ${projectName}.x)

## Project is using only C++
project(${projectName} CXX)

set(HEADERS ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetMakePhysSignal/SDAMakePhysSignals.h
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetMatchHits/SDAMatchHits.h
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetMatchLORs/SDAMatchLORs.h
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoAmplitudeCalc/SDARecoAmplitudeCalc.h
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoChargeCalc/SDARecoChargeCalc.h
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoDrawAllCharges/SDARecoDrawAllCharges.h
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoDrawAllOffsets/SDARecoDrawAllOffsets.h
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoOffsetCalc/SDARecoOffsetsCalc.h
            ${PROJECT_SOURCE_DIR}/../modules/tools/JPetHitTools/FindConstant.h
            ${PROJECT_SOURCE_DIR}/../modules/tools/JPetRecoSignalTools/JPetRecoSignalTools.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetMakePhysSignal/SDAMakePhysSignals.cpp
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetMatchHits/SDAMatchHits.cpp
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetMatchLORs/SDAMatchLORs.cpp
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoAmplitudeCalc/SDARecoAmplitudeCalc.cpp
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoChargeCalc/SDARecoChargeCalc.cpp
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoDrawAllCharges/SDARecoDrawAllCharges.cpp
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoDrawAllOffsets/SDARecoDrawAllOffsets.cpp
            ${PROJECT_SOURCE_DIR}/../modules/SDA/JPetRecoOffsetCalc/SDARecoOffsetsCalc.cpp
            ${PROJECT_SOURCE_DIR}/../modules/tools/JPetHitTools/FindConstant.cpp
            ${PROJECT_SOURCE_DIR}/../modules/tools/JPetRecoSignalTools/JPetRecoSignalTools.cpp)


################################################################################
## Build definitions and libraries linking
add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)
target_include_directories(${projectBinary} PRIVATE
                           $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../modules/SDA/>
                           $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/../modules/tools/>)

## Copy the auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
