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
            ${CMAKE_CURRENT_SOURCE_DIR}/UniversalFileLoader.h)

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
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)


add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

add_custom_target(clean_data_largebarrelextended
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

## Create variable for list with depends files path
set(copy_depends)

################################################################################
## Copy the example auxiliary files
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
add_custom_target(copy_files DEPENDS ${copy_depends})
## Add dependencies between building project and copy files
add_dependencies(${projectBinary} copy_files)

################################################################################
## Unit tests
option(PACKAGE_TESTS "Build the tests" ON)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
