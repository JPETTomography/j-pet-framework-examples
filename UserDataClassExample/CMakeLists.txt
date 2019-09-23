################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Extension of the LargeBarrelAnalysis example
##   demonstrating adding a custom user-provided data class
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()


################################################################################
## Project name
set(projectName UserDataClassExample)

## Auxiliary files
set(AUXILIARY_FILES
  README.md
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX)

## Using source files of modules from LargeBarrelAnalysis
set(use_modules_from ../LargeBarrelAnalysis)

set(HEADERS ${use_modules_from}/EventCategorizer.h
            ${use_modules_from}/EventCategorizerTools.h
            ${use_modules_from}/EventFinder.h
            ${use_modules_from}/HitFinder.h
            ${use_modules_from}/HitFinderTools.h
            ${use_modules_from}/SignalFinder.h
            ${use_modules_from}/SignalFinderTools.h
            ${use_modules_from}/SignalTransformer.h
            ${use_modules_from}/TimeWindowCreator.h
            ${use_modules_from}/TimeWindowCreatorTools.h
            ${use_modules_from}/UniversalFileLoader.h
            ${use_modules_from}/ToTConverter.h
            ${CMAKE_CURRENT_SOURCE_DIR}/LORFinder.h
            ${CMAKE_CURRENT_SOURCE_DIR}/JPetLORevent.h)
          
set(SOURCES ${use_modules_from}/EventCategorizer.cpp
            ${use_modules_from}/EventCategorizerTools.cpp
            ${use_modules_from}/EventFinder.cpp
            ${use_modules_from}/HitFinder.cpp
            ${use_modules_from}/HitFinderTools.cpp
            ${use_modules_from}/SignalFinder.cpp
            ${use_modules_from}/SignalFinderTools.cpp
            ${use_modules_from}/SignalTransformer.cpp
            ${use_modules_from}/TimeWindowCreator.cpp
            ${use_modules_from}/TimeWindowCreatorTools.cpp
            ${use_modules_from}/UniversalFileLoader.cpp
            ${use_modules_from}/ToTConverter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/LORFinder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/JPetLORevent.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

set(DICTIONARY_NAME G__UserDataClasses)
set(HEADERS_WITH_DICTIONARY_REQUIRED
  ${CMAKE_CURRENT_SOURCE_DIR}/JPetLORevent.h
  )

set(LINKDEF_NAME LinkDef.h)

get_property(DICTIONARY_INCLUDE_DIRS TARGET JPetFramework::JPetFramework PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
foreach(dir ${DICTIONARY_INCLUDE_DIRS})
  list(APPEND DICTIONARY_INCLUDES -I${dir})
endforeach()

include_directories(ROOT_BUG) # Fix bug in NewMacros Root file
ROOT_GENERATE_DICTIONARY(${DICTIONARY_NAME} ${HEADERS_WITH_DICTIONARY_REQUIRED}
  LINKDEF ${LINKDEF_NAME}
  OPTIONS ${DICTIONARY_INCLUDES})
          
add_executable(${projectBinary} ${SOURCES} ${HEADERS} ${DICTIONARY_NAME})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework
                                       Boost::program_options)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

################################################################################
## Copy the example auxiliary files
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

