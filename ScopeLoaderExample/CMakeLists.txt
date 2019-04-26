################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Loading scope data project.
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

################################################################################
## Project name
set(projectName ScopeLoaderExample)

################################################################################
## Auxiliary files
set(AUXILIARY_FILES
  README.md
  run.sh
  cfg
)

################################################################################
## Binary, header and source files and additional sources definitions
set(projectBinary ${projectName}.x)

## Project is using only C++
project(${projectName} CXX)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${projectBinary} ${SOURCES})
target_link_libraries(${projectBinary} JPetFramework::JPetFramework)

################################################################################
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
