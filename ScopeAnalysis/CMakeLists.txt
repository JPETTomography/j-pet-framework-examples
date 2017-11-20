# J-PET tool project using J-PET framework
#

cmake_minimum_required(VERSION 2.6)

######################################################################
### when creating a new project,
### set this section appropriately for your project
######################################################################
set(projectName ScopeAnalysis)

set(AUXILLIARY_FILES
  )

######################################################################
### this section should not need to be modified for a new project
######################################################################
set(projectBinary ${projectName}.x)

project(${projectName} CXX) # using only C++

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

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary} JPetFramework)

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
