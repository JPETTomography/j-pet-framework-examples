set(projectName JPetRecoImageTools)


######################################################################
### this section should not need to be modified for a new project
######################################################################
set(projectBinary ${projectName}.x)

project(${projectName} CXX) # using only C++

file(GLOB SOURCES JPet*/*.cpp)
file(GLOB UNIT_TEST_SOURCES JPet*/*Test.cpp)
list(REMOVE_ITEM SOURCES ${UNIT_TEST_SOURCES})

include_directories(${Framework_INCLUDE_DIRS})
add_definitions(${Framework_DEFINITIONS})

find_package(FFTW REQUIRED)
include_directories(${FFTW_INCLUDE_DIRS})

# unit tests
set(TESTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})
foreach(test_source ${UNIT_TEST_SOURCES})
  get_filename_component(test ${test_source} NAME_WE)
  generate_root_dictionaries(test_dictionaries SOURCES ${test_source})
  list(APPEND test_binaries ${test}.x)
  add_executable(${test}.x EXCLUDE_FROM_ALL ${test_source} ${SOURCES}
    ${test_dictionaries}
    )
  set_target_properties(${test}.x PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR} )
  target_link_libraries(${test}.x
    JPetFramework
    ${Boost_LIBRARIES}
    ${FFTW_LIBRARIES}
    )
endforeach()

add_custom_target(tests_create_ln DEPENDS ${test_binaries} ${TESTS_DIR}/unitTestData)
add_custom_command(OUTPUT ${TESTS_DIR}/unitTestData
  COMMAND ln -s ${CMAKE_SOURCE_DIR}/unitTestData ${TESTS_DIR}/unitTestData
)

add_custom_target(tests_large_barrel DEPENDS ${test_binaries} )
