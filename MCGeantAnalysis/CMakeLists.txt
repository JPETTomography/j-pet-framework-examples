################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Builds project for analysis of Monte Carlo generated data
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

################################################################################
## Project name
set(projectName MCGeantAnalysis)

## Auxiliary files
set(AUXILLIARY_FILES
  userParams.json
  README.md
  PARAMETERS.md
  run.sh
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX)


set(use_modules_from ../LargeBarrelAnalysis)
set(HEADERS ${use_modules_from}/HitFinder.h
            ${use_modules_from}/HitFinderTools.h
            ${use_modules_from}/EventFinder.h
            ${use_modules_from}/EventCategorizer.h
            ${use_modules_from}/EventCategorizerTools.h
            ${use_modules_from}/UniversalFileLoader.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${use_modules_from}/HitFinder.cpp
            ${use_modules_from}/HitFinderTools.cpp
            ${use_modules_from}/EventFinder.cpp
            ${use_modules_from}/EventCategorizer.cpp
            ${use_modules_from}/EventCategorizerTools.cpp
            ${use_modules_from}/UniversalFileLoader.cpp)

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)

################################################################################
## Copy the example auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
