################################################################################
## Data analysis project based on J-PET Framework
## Created by J-PET Framework developers 2016-2020
##
## Description:
##   Event Classification from reconstructed data
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()


################################################################################
## Project name
set(projectName EventClassification)

## Auxiliary files
#set(AUXILIARY_FILES
  #userParams.json
  #PARAMETERS.md
  #README.md
  #run.sh
#)

################################################################################
## Binary, header and source files definitions
set(projectBinary ${projectName}.x)
project(${projectName} CXX)

set(HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizerTools.h
            ${CMAKE_CURRENT_SOURCE_DIR}/EventFinder.h
)

set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/EventCategorizerTools.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/EventFinder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_executable(${projectBinary} ${SOURCES} ${HEADERS})
target_link_libraries(
  ${projectBinary} JPetFramework::JPetFramework Boost::program_options
)

add_custom_target(
  clean_data_${projectName} COMMAND rm -f *.root
  )

################################################################################
## Copy the example auxiliary files
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${AUXILIARY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

################################################################################
## Unit tests
#option(PACKAGE_TESTS "Build the tests" ON)
#if(PACKAGE_TESTS)
    #enable_testing()
    #add_subdirectory(tests)
#endif()
