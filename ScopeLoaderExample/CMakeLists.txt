################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Loading scope data project.
################################################################################

cmake_minimum_required(VERSION 2.6)

################################################################################
## Project name
set(projectName ScopeLoaderExample)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  run.sh
  README
  cfg
  )

################################################################################
## Binary, header and source files and additional sources definitions
set(projectBinary ${projectName}.x)

## Project is using only C++
project(${projectName} CXX)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

## Copy the auxiliary files
foreach(file_i ${AUXILIARY_FILES})
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
endforeach(file_i)
