# J-PET tool project using J-PET framework
#
# Author: Adam Strzelecki <adam.strzelecki@uj.edu.pl>
#
# Description:
#   Builds project depending on J-PET framework.

cmake_minimum_required(VERSION 2.6)

set(projectName AnalysisExample)

project(${projectName} CXX) # using only C++

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

generate_root_dictionaries(DICTIONARIES SOURCES ${SOURCES})
add_executable(${projectName} ${SOURCES} ${HEADERS} ${DICTIONARIES})
target_link_libraries(${projectName} JPetFramework)

add_custom_target(clean_data
  COMMAND rm -f *.tslot.*.root *.phys.*.root *.sig.root)

# copy the DB config file to an expected location
set(db_config_dir ${CMAKE_CURRENT_BINARY_DIR}/DBConfig)

add_custom_command(TARGET ${projectName}
  POST_BUILD
  COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/../j-pet-framework/DBHandler/Config ${db_config_dir}
  )

