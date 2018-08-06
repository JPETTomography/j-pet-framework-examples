# J-PET tool project using J-PET framework
#
# Author: Adam Strzelecki <adam.strzelecki@uj.edu.pl>
#
# Description:
#   Builds project depending on J-PET framework.

cmake_minimum_required(VERSION 2.6)

set(projectName ImageReconstruction)

set(projectBinary ${projectName}.x)

project(${projectName} CXX) # using only C++

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
file(GLOB MAIN_CPP main.cpp)

file(GLOB UNIT_TEST_SOURCES *Test.cpp)
list(REMOVE_ITEM SOURCES ${UNIT_TEST_SOURCES})

file(GLOB SOURCES_WITHOUT_MAIN *.cpp)
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${UNIT_TEST_SOURCES})
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${MAIN_CPP})

# add options from j-pet-mlem as compile flags removing PET_ prefix
# j-pet-mlem set some parameters of the build by CMake using definitions, but they are limited to project
# so I copy them here
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

# add dependencies from j-pet-mlem
set(use_modules_from ../j-pet-mlem)
list(APPEND HEADERS ${use_modules_from}/src/util/png_writer.h)
list(APPEND SOURCES ${use_modules_from}/src/util/png_writer.cpp)

# libpng for output image generation
find_package(PNG)
if(NOT PNG_FOUND AND MSVC)
  include(ExtPNG) # builds libpng automatically
endif()
if(PNG_FOUND)
  add_definitions(-DHAVE_LIBPNG=1 ${PNG_DEFINITIONS})
  include_directories(${PNG_INCLUDE_DIRS})
  link_libraries(${PNG_LIBRARIES})
endif()

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework ${PNG_LIBRARIES})

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

# unit tests
set(TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})
foreach(test_source ${UNIT_TEST_SOURCES})
  get_filename_component(test ${test_source} NAME_WE)
  generate_root_dictionaries(test_dictionaries SOURCES ${test_source})
  list(APPEND test_binaries ${test}.x)
  add_executable(${test}.x EXCLUDE_FROM_ALL ${test_source} ${SOURCES_WITHOUT_MAIN}
    ${test_dictionaries}
    )
  set_target_properties(${test}.x PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR} )
  target_link_libraries(${test}.x
    JPetFramework
    ${Boost_LIBRARIES}
    )
endforeach()

add_custom_target(tests_ImageReconstruction DEPENDS ${test_binaries} )

