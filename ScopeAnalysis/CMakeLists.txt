################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Analysis of the scope data.
################################################################################

cmake_minimum_required(VERSION 2.6)

################################################################################
## Project name
set(projectName ScopeAnalysis)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  README.md
  run.sh
)

################################################################################
## Binary, header and source files and additional sources definitions
set(projectBinary ${projectName}.x)

## Project is using only C++
project(${projectName} CXX)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
file(GLOB MAIN_CPP main.cpp)

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

set(ADDITIONAL_SOURCES
  ../modules/SDA
  ../modules/tools
)

foreach(dir ${ADDITIONAL_SOURCES})
  file(GLOB HEADERS_TMP ${dir}/JPet*/*.h)
  file(GLOB SOURCES_TMP ${dir}/JPet*/*.cpp)
  file(GLOB FOLDERS_TMP ${dir})
  list(REMOVE_ITEM FOLDERS_TMP ${HEADERS_TMP} ${SOURCES_TMP})
  list(APPEND HEADERS ${HEADERS_TMP})
  list(APPEND SOURCES ${SOURCES_TMP})
  list(APPEND FOLDERS ${FOLDERS_TMP})
endforeach()
include_directories(${FOLDERS})

################################################################################
## Build definitions and libraries linking
add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

## Copy the auxiliary files
foreach(file_i ${AUXILIARY_FILES})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${file_i})
    set(CP_CMD copy_directory)
  else()
    set(CP_CMD copy)
  endif()
  add_custom_command(TARGET ${projectBinary} POST_BUILD COMMAND ${CMAKE_COMMAND}
    ARGS -E ${CP_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/${file_i} ${CMAKE_CURRENT_BINARY_DIR}/${file_i}
  )
endforeach(file_i)
