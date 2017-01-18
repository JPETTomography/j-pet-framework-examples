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
set(projectName effectiveVelocity)

set(AUXILLIARY_FILES
  run.sh
  conf_trb3.xml
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

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_executable(${projectBinary} ${SOURCES} ${HEADERS} ${DICTIONARIES})
target_link_libraries(${projectBinary} JPetFramework)

add_custom_target(clean_data_velocity
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

# copy the DB config file to an expected location
set(db_config_dir ${CMAKE_CURRENT_BINARY_DIR}/../DBConfig)

add_custom_command(TARGET ${projectBinary}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../j-pet-framework/DBHandler/Config ${db_config_dir}
)

# copy the example auxilliary files
foreach( file_i ${AUXILLIARY_FILES})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${file_i})
    set(CP_CMD copy_directory)
  else()
    set(CP_CMD copy)
  endif()

  add_custom_command(
    TARGET ${projectBinary}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
    )
endforeach( file_i )

# copy the ROOT scripts
foreach( file_i ${ROOT_SCRIPTS})
add_custom_command(
  TARGET ${projectBinary}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
  )
endforeach( file_i )
