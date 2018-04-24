# J-PET tool project using J-PET framework
#
# Author: Adam Strzelecki <adam.strzelecki@uj.edu.pl>
#
# Description:
#   Builds project depending on J-PET framework.

cmake_minimum_required(VERSION 2.6)

######################################################################
### when creating a new project,
### set this section appropriately for your project
######################################################################
set(projectName NewProjectName)

# files to be copied from some other example directory, if any
set(USE_AUX_FILES_FROM LargeBarrelAnalysis)

set(AUXILLIARY_FILES
  run.sh
  userParams.json
  README
  )

set(ROOT_SCRIPTS
  rootlogon.C
  rootalias.C
  )

set(projectBinary ${projectName}.x)

project(${projectName} CXX) # using only C++

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

######################################################################
### Use source files of modules from LargeBarrelAnalysis
###
### If you are using your own version of some module which
### is also presenr in LargeBarrelAnalysis,
### comment out the respective lines below so as not to use
### this module's source from LargeBarrelAnalysis.
######################################################################
set(use_modules_from ../LargeBarrelAnalysis)
#
list(APPEND HEADERS ${use_modules_from}/TimeWindowCreator.h)
list(APPEND SOURCES ${use_modules_from}/TimeWindowCreator.cpp)
list(APPEND HEADERS ${use_modules_from}/UniversalFileLoader.h)
list(APPEND SOURCES ${use_modules_from}/UniversalFileLoader.cpp)
#
list(APPEND HEADERS ${use_modules_from}/SignalFinder.h)
list(APPEND SOURCES ${use_modules_from}/SignalFinder.cpp)
list(APPEND HEADERS ${use_modules_from}/SignalFinderTools.h)
list(APPEND SOURCES ${use_modules_from}/SignalFinderTools.cpp)
#
list(APPEND HEADERS ${use_modules_from}/SignalTransformer.h)
list(APPEND SOURCES ${use_modules_from}/SignalTransformer.cpp)
#
list(APPEND HEADERS ${use_modules_from}/HitFinder.h)
list(APPEND SOURCES ${use_modules_from}/HitFinder.cpp)
list(APPEND HEADERS ${use_modules_from}/HitFinderTools.h)
list(APPEND SOURCES ${use_modules_from}/HitFinderTools.cpp)
#
list(APPEND HEADERS ${use_modules_from}/EventFinder.h)
list(APPEND SOURCES ${use_modules_from}/EventFinder.cpp)
#
list(APPEND HEADERS ${use_modules_from}/EventCategorizer.h)
list(APPEND SOURCES ${use_modules_from}/EventCategorizer.cpp)
list(APPEND HEADERS ${use_modules_from}/EventCategorizerTools.h)
list(APPEND SOURCES ${use_modules_from}/EventCategorizerTools.cpp)

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

set(copy_depends) #create variable for list with depends files path

# copy the example auxilliary files
foreach( file_i ${AUXILLIARY_FILES})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})

  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../${USE_AUX_FILES_FROM}/${file_i})
    set(CP_CMD copy_directory)
  else()
    set(CP_CMD copy)
  endif()
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/../${USE_AUX_FILES_FROM}/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    )
endforeach( file_i )

# copy the ROOT scripts
foreach( file_i ${ROOT_SCRIPTS})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    )
endforeach( file_i )

add_custom_target(copy_files_${projectName} DEPENDS ${copy_depends}) #add new target that depends on copied files
add_dependencies(${projectBinary} copy_files) #add dependencies between building project and copy files
