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
set(projectName MCGeantAnalysis)

set(AUXILLIARY_FILES
  README.md
  run_mc.sh
  run_data.sh
  calib_run_3/conf_trb3.xml
  calib_run_3/userParams.json
  calib_run_3/detectorSetupRun2345.json
  calib_run_3/run2_thirdAttempt_Sync9.db
  calib_run_3/TimeConstants_Run3.txt
  calib_run_3/tot_calib_dabc17083123644_a.root
  calib_run_3/resultsForThresholda.txt
  )

set(ROOT_SCRIPTS
  rootlogon.C
  rootalias.C
  )

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


include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

set(copy_depends) #create variable for list with depends files path

# copy the example auxilliary files
foreach( file_i ${AUXILLIARY_FILES})
    make_directory(calib_run_3)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i} COPYONLY)
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
