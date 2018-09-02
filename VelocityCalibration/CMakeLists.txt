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
set(projectName VelocityCalibration)

set(AUXILLIARY_FILES
  run.sh
  userParams.json
  README
  )

set(ROOT_SCRIPTS
  rootlogon.C
  rootalias.C
  )

######################################################################
### this section should not need to be modified for a new project
######################################################################
set(projectBinary ${projectName}.x)

project(${projectName} CXX) # using only C++

file(GLOB HEADERS *.h ../LargeBarrelAnalysis/*.h)
file(GLOB SOURCES *.cpp ../LargeBarrelAnalysis/*.cpp)
file(GLOB LBAE_MAIN_CPP ../LargeBarrelAnalysis/main.cpp)
file(GLOB MAIN_CPP main.cpp)
file(GLOB UNIT_TEST_LBAE_SOURCES ../LargeBarrelAnalysis/*Test.cpp)
file(GLOB ESTVEL_SOURCE estimateVelocity.cpp)

file(GLOB SOURCES_WITHOUT_MAIN *.cpp)
list(REMOVE_ITEM SOURCES ${LBAE_MAIN_CPP})
list(REMOVE_ITEM SOURCES ${ESTVEL_SOURCE})
list(REMOVE_ITEM SOURCES ${UNIT_TEST_LBAE_SOURCES})
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${MAIN_CPP})
list(REMOVE_ITEM SOURCES_WITHOUT_MAIN ${LBAE_MAIN_CPP})

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS} )

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

add_executable(estimateVelocity ${ESTVEL_SOURCE})
target_link_libraries(estimateVelocity JPetFramework)

add_custom_target(clean_data_velocitycalibration
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

set(copy_depends) #create variable for list with depends files path

# copy the example auxilliary files
foreach( file_i ${AUXILLIARY_FILES})
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

# copy the ROOT scripts
foreach( file_i ${ROOT_SCRIPTS})
  list(APPEND copy_depends ${CMAKE_CURRENT_BINARY_DIR}/${file_i})
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    )
endforeach( file_i )

set(TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})
add_custom_command(OUTPUT ${TESTS_DIR}/unitTestData
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/unitTestData ${TESTS_DIR}/unitTestData
)

add_custom_target(copy_files_velocity DEPENDS ${copy_depends} ${TESTS_DIR}/unitTestData) #add new target that depends on copied files

add_dependencies(${projectBinary} copy_files_velocity) #add dependencies between building project and copy files

