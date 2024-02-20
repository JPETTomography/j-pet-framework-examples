################################################################################
## Set of projects based on J-PET Framework
## Created by J-PET Framework developers 2016-2018
##
## Description:
##   Script for building J-PET Framework, J-PET MLEM and all the examples
################################################################################

cmake_minimum_required(VERSION 3.1...3.14)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.14)
endif()

project(JPetFrameworkExamples VERSION 1.0.0
                  LANGUAGES CXX)
set(PROJECT_DESCRIPTION "JPetFrameworkExamples module")

message(STATUS "")
message(STATUS "    == ${PROJECT_NAME} Project configuration ==")
message(STATUS "")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_PLATFORM_INDEPENDENT_CODE ON)

# Force out-of-source build
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Be nice and export compile commands by default, this is handy for clang-tidy
# and for other tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Helpful option enable build profiling to identify slowly compiling files
option(MEASURE_ALL "When enabled all commands will be passed through time command" OFF)
if(MEASURE_ALL)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "time")
endif()

################################################################################
## Find Threads package
#
find_package(Threads REQUIRED)

################################################################################
## Find BOOST LIBs

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.50 REQUIRED
             COMPONENTS filesystem
                        program_options
                        regex
                        system
                        log_setup
                        log
                        date_time
                        thread
                        chrono
                        atomic
                        )

if(NOT TARGET Boost::filesystem)
    add_library(Boost::filesystem IMPORTED INTERFACE)
    set_property(TARGET Boost::filesystem PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
    set_property(TARGET Boost::filesystem PROPERTY
        INTERFACE_LINK_LIBRARIES ${Boost_LIBRARIES})
endif()

################################################################################
## Find CERN ROOT 6.10+
find_package(ROOT CONFIG REQUIRED)
if(ROOT_FOUND)
  message(STATUS "ROOT (version ${ROOT_VERSION}) was found using ROOTConfig")
endif()

# the RootNewMacros.cmake file has to be found and included manually
# only for ROOT versions prior to 6.20
if(ROOT_VERSION VERSION_LESS "6.20")
  if(EXISTS "${ROOT_DIR}/modules/RootNewMacros.cmake")
    include("${ROOT_DIR}/modules/RootNewMacros.cmake")
    message(STATUS "Found RootNewMacros.cmake in: ${ROOT_DIR}/modules!")
  elseif(EXISTS "${ROOT_DIR}/cmake/modules/RootNewMacros.cmake")
    include("${ROOT_DIR}/cmake/modules/RootNewMacros.cmake")
    message(STATUS "Found RootNewMacros.cmake in: ${ROOT_DIR}/cmake/modules!")
  elseif(EXISTS "${ROOT_DIR}/../cmake/modules/RootNewMacros.cmake")
    include("${ROOT_DIR}/../cmake/modules/RootNewMacros.cmake")
    message(STATUS "Found RootNewMacros.cmake in: ${ROOT_DIR}/../cmake/modules!")
  else()
    message("ERROR! Could not find RootNewMacros.cmake file! Aborting..")
    return(1)
  endif()
endif()

# fix missing include directories property in versions < ROOT 6.12
set_target_properties(ROOT::Core PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${ROOT_INCLUDE_DIRS}")


add_library(ROOT::Flags_CXX IMPORTED INTERFACE)

# Fix for ROOT_CXX_FLAGS not actually being a CMake list
separate_arguments(ROOT_CXX_FLAGS)
set_property(TARGET ROOT::Flags_CXX APPEND PROPERTY
    INTERFACE_COMPILE_OPTIONS ${ROOT_CXX_FLAGS})

# Add definitions
separate_arguments(ROOT_DEFINITIONS)
foreach(_flag ${ROOT_EXE_LINKER_FLAG_LIST})
    # Remove -D or /D if present
    string(REGEX REPLACE [=[^[-//]D]=] "" _flag ${_flag})
    set_property(TARGET ROOT::Flags APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${_flag})
endforeach()

# This also fixes a bug in the linker flags
string(REPLACE "-L " "-L" ROOT_EXE_LINKER_FLAGS "${ROOT_EXE_LINKER_FLAGS}")
separate_arguments(ROOT_EXE_LINKER_FLAGS)
set_property(TARGET ROOT::Flags_CXX APPEND PROPERTY
    INTERFACE_LINK_LIBRARIES ${ROOT_EXE_LINKER_FLAGS})

################################################################################
## Find JPetFramework
find_package(JPetFramework CONFIG QUIET)
if(JPetFramework_FOUND)
  message(STATUS "JPetFramework (version ${JPetFramework_VERSION}) was found using JPetFrameworkConfig")
else()
  message(STATUS "Could not find JPetFramework, please source path/to/framework/bin/thisframework.sh to set correct paths..")
  find_package(JPetFramework CONFIG REQUIRED) #to print error message
endif(JPetFramework_FOUND)
################################################################################
## Download input and configuration files
## The script shouldn't do anything if the data is present and correct.
option(DOWNLOAD_DATA "Download data" ON)
if(DOWNLOAD_DATA)
  set(DOWNLOAD_BASE_PATH ${PROJECT_SOURCE_DIR})

  file(DOWNLOAD "http://sphinx.if.uj.edu.pl/framework/examples_reformed.sha" ${DOWNLOAD_BASE_PATH}/examples_reformed.sha)
  file(READ ${DOWNLOAD_BASE_PATH}/examples_reformed.sha SHA_HASHES)
  string(REGEX REPLACE "\n" ";" SHA_HASHES "${SHA_HASHES}") #create cmake array from hash + file path
  foreach(SHA_HASH ${SHA_HASHES})
    string(REGEX REPLACE "  " ";" SHA_HASH "${SHA_HASH}") #Split hash and file path
    list(GET SHA_HASH 0 SHA256) #get sha hash
    list(GET SHA_HASH 1 PATH) #get file path
    string(REPLACE "ExamplesReformed/" "" SAVE_PATH ${PATH})

    if(EXISTS "${DOWNLOAD_BASE_PATH}/${SAVE_PATH}")
      file(SHA256 ${DOWNLOAD_BASE_PATH}/${SAVE_PATH} CURRENT_SHA256)
    endif(EXISTS "${DOWNLOAD_BASE_PATH}/${SAVE_PATH}")
    if(NOT "${CURRENT_SHA256}" STREQUAL "${SHA256}")
      message(STATUS "SHA value of ${DOWNLOAD_BASE_PATH}/${SAVE_PATH} to not match! Downloading file..")
      file(DOWNLOAD "http://sphinx.if.uj.edu.pl/framework/${PATH}" ${DOWNLOAD_BASE_PATH}/${SAVE_PATH} EXPECTED_HASH SHA256=${SHA256})
    endif(NOT "${CURRENT_SHA256}" STREQUAL "${SHA256}")
  endforeach(SHA_HASH ${SHA_HASHES})
  file(REMOVE ${DOWNLOAD_BASE_PATH}/examples_reformed.sha)
endif()

################################################################################
## Directories with examples
add_subdirectory(ModularDetectorAnalysis)
add_subdirectory(ListmodeExport)
add_subdirectory(MCGeantAnalysis)
add_subdirectory(RedModuleAnalysis)
add_subdirectory(opsAnalysis)
#add_subdirectory(Imaging)
#add_subdirectory(InterfileReconstruction)
#add_subdirectory(LargeBarrelAnalysis)
#add_subdirectory(InterThresholdCalibration)
#add_subdirectory(VelocityCalibration)
#add_subdirectory(NewAnalysisTemplate)
#add_subdirectory(ImageReconstruction)
#add_subdirectory(ScopeLoaderExample)
#add_subdirectory(TimeCalibration)
#add_subdirectory(PhysicAnalysis)
#add_subdirectory(CosmicAnalysis)
#add_subdirectory(ScopeAnalysis)
#add_subdirectory(TimeCalibration_iter)
#add_subdirectory(UserDataClassExample)
