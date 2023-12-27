cmake_minimum_required(VERSION 3.0.0)

include(CPM.cmake)

set(CMAKE_CXX_STANDARD 17)
project(vanilla CXX)

set(CMAKE_GENERATOR_PLATFORM x64)

CPMAddPackage("gh:geode-sdk/Broma#169af29")
CPMAddPackage("gh:fmtlib/fmt#9.1.0")

file(GLOB SOURCES *.cpp Broma/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

set(CMAKE_SIZEOF_VOID_P 4)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target i386-pc-windows-msvc")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target i386-pc-windows-msvc")
  add_definitions("--target=i386-pc-windows-msvc")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt Broma)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})