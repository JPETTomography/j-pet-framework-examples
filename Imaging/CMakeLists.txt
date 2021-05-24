################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Imaging stream project.
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

################################################################################
## Project name
set(projectName Imaging)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  userParams.json
  PARAMETERS.md
  README.md
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)

## Project is using only C++
project(${projectName} CXX)

set(use_modules_from ../ModularDetectorAnalysis)
set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizerImaging.h
            ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizerTools.h
            ${use_modules_from}/TimeWindowCreator.h
            ${use_modules_from}/TimeWindowCreatorTools.h
            ${use_modules_from}/SignalFinder.h
            ${use_modules_from}/SignalFinderTools.h
            ${use_modules_from}/SignalTransformer.h
            ${use_modules_from}/HitFinder.h
            ${use_modules_from}/HitFinderTools.h
            ${use_modules_from}/EventFinder.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizerImaging.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizerTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${use_modules_from}/TimeWindowCreator.cpp
            ${use_modules_from}/TimeWindowCreatorTools.cpp
            ${use_modules_from}/SignalFinder.cpp
            ${use_modules_from}/SignalFinderTools.cpp
            ${use_modules_from}/SignalTransformer.cpp
            ${use_modules_from}/HitFinder.cpp
            ${use_modules_from}/HitFinderTools.cpp
            ${use_modules_from}/EventFinder.cpp)

################################################################################
## Build definitions and libraries linking

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root
)

################################################################################
## Copy the auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

################################################################################
### Add new target that depends on copied files
#add_custom_target(copy_files_${projectName} DEPENDS ${copy_depends})
### Add dependencies between building project and copy files
#add_dependencies(${projectBinary} copy_files_${projectName})

################################################################################
## Unit tests
#option(PACKAGE_TESTS "Build the tests" ON)
#if(PACKAGE_TESTS)
    #enable_testing()
    #add_subdirectory(tests)
#endif()
