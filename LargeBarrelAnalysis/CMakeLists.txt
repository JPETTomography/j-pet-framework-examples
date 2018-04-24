################################################################################
## J-PET tool project using J-PET framework
##
## Author: Adam Strzelecki <adam.strzelecki@uj.edu.pl>
##
## Description:
##   Builds project depending on J-PET framework.
################################################################################

cmake_minimum_required(VERSION 2.6)

################################################################################
## Project name
set(projectName LargeBarrelAnalysis)

set(AUXILIARY_FILES
  run.sh
  README
  run.sh
)

set(ROOT_SCRIPTS
  rootlogon.C
  rootalias.C
)

################################################################################
## Build targets
set(projectBinary ${projectName}.x)
project(${projectName} CXX)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
file(GLOB MAIN_CPP main.cpp)
file(GLOB UNIT_TEST_SOURCES *Test.cpp)
list(REMOVE_ITEM SOURCES ${UNIT_TEST_SOURCES})
file(GLOB SOURCES_WITHOUT_MAIN *.cpp)
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${UNIT_TEST_SOURCES})
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${MAIN_CPP})

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

add_custom_target(clean_data_largebarrelextended
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

set(copy_depends) #create variable for list with depends files path

################################################################################
## Copy the example auxiliary files
foreach( file_i ${AUXILIARY_FILES})
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

################################################################################
## Copy the ROOT scripts
foreach( file_i ${ROOT_SCRIPTS})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    )
endforeach( file_i )

################################################################################
## Add new target that depends on copied files
add_custom_target(copy_files DEPENDS ${copy_depends})
## Add dependencies between building project and copy files
add_dependencies(${projectBinary} copy_files)

################################################################################
## Unit tests
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

add_custom_target(tests_LargeBarrel DEPENDS ${test_binaries} )
