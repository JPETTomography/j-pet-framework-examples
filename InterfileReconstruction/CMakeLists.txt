cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()


################################################################################
## Project name
set(projectName InterfileReconstruction)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/../ModularDetectorAnalysis/EventCategorizerTools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/InterfileMLPReconstruction.h)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../ModularDetectorAnalysis/EventCategorizerTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/InterfileMLPReconstruction.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(${projectBinary}
  ${DICTIONARY_LIB}
  JPetFramework::JPetFramework
  Boost::program_options)

target_include_directories(${projectBinary} PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../ModularDetectorAnalysis>)

add_custom_target(clean_data_${projectName}
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)
