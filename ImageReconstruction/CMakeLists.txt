################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Image reconstruction for Large Barrel setup
################################################################################

cmake_minimum_required(VERSION 2.6)

################################################################################
## Project name
set(projectName ImageReconstruction)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  userParams.json
  PARAMETERS.md
  README.md
  run.sh
)

set(ROOT_SCRIPTS
  rootlogon.C
  rootalias.C
)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX)

include_directories(../modules/large_barrel/JPetRecoImageTools/)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
file(GLOB MAIN_CPP main.cpp)
file(GLOB UNIT_TEST_SOURCES *Test.cpp)
list(REMOVE_ITEM SOURCES ${UNIT_TEST_SOURCES})
file(GLOB SOURCES_WITHOUT_MAIN *.cpp)
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${UNIT_TEST_SOURCES})
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${MAIN_CPP})

################################################################################
## Add options from j-pet-mlem as compile flags removing PET_ prefix
## j-pet-mlem sets some parameters of the CMake build by using definitions,
## that are limited to project, it is necessary to copy them here
get_cmake_property(vars VARIABLES)
foreach(var ${vars})
  if(var MATCHES "^PET_" AND NOT var MATCHES "_DIR$")
    string(REGEX REPLACE "^PET_" "" name ${var})
    if(name MATCHES "_TYPE$")
      string(REGEX REPLACE "_TYPE$" "" type_name ${name})
      add_definitions(-DUSE_${${var}}_${type_name}=1)
      add_definitions(-D${name}=${${var}})
    elseif(${var})
      if(${var} STREQUAL ON)
        add_definitions(-D${name}=1)
      else()
        add_definitions(-D${name}=${${var}})
      endif()
    else()
      add_definitions(-D${name}=0)
    endif()
  endif()
endforeach()

## Add dependencies from j-pet-mlem
set(use_modules_from ../j-pet-mlem)
list(APPEND HEADERS ${use_modules_from}/src/util/png_writer.h)
list(APPEND SOURCES ${use_modules_from}/src/util/png_writer.cpp)

## libpng for output image generation
find_package(PNG)
if(NOT PNG_FOUND AND MSVC)
  include(ExtPNG)
endif()
if(PNG_FOUND)
  add_definitions(-DHAVE_LIBPNG=1 ${PNG_DEFINITIONS})
  include_directories(${PNG_INCLUDE_DIRS})
  link_libraries(${PNG_LIBRARIES})
endif()

################################################################################
## Include J-PET Framework
include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework ${PNG_LIBRARIES} JPetRecoImageTools)

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
## Unit tests
set(TEST_SRC SinogramCreatorTools.cpp)
set(TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})
foreach(test_source ${UNIT_TEST_SOURCES})
  get_filename_component(test ${test_source} NAME_WE)
  list(APPEND test_binaries ${test}.x)
  add_executable(${test}.x EXCLUDE_FROM_ALL ${test_source} ${TEST_SRC}
    ${test_dictionaries}
    )
  set_target_properties(${test}.x PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR} )
  target_link_libraries(${test}.x
    JPetFramework
    ${Boost_LIBRARIES}
    ${FFTW_LIBRARIES}
    )
endforeach()

add_custom_target(tests_ImageReconstruction DEPENDS ${test_binaries})

################################################################################
## Add new target that depends on copied files
add_custom_target(copy_files_${projectName} DEPENDS ${copy_depends})
## Add dependencies between building project and copy files
add_dependencies(${projectBinary} copy_files_${projectName})
