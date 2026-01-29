cmake_minimum_required (VERSION 3.10)

project(tplmp)

add_library(${PROJECT_NAME} INTERFACE)

set(PROJECT_SOURCE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_include_directories(${PROJECT_NAME} INTERFACE "${PROJECT_SOURCE_INCLUDE_DIRS}")
 
set(PROJECT_INSTALL_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
set(PROJECT_INSTALL_LIBRARIES)

install(DIRECTORY "${PROJECT_SOURCE_INCLUDE_DIRS}/" DESTINATION "include")
message(STATUS "set copy path from \"${PROJECT_SOURCE_INCLUDE_DIRS}\" to \"${PROJECT_INSTALL_INCLUDE_DIRS}\"")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.cmake.in" "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}Config.cmake" @ONLY)
message(STATUS "generated ${PROJECT_NAME}Config.cmake in: \"${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}Config.cmake\"")

message(STATUS "configure complete, run cmake --build . --target install to copy include directory")