cmake_minimum_required(VERSION 3.16)

project(dlms_wrapper
  VERSION 0.1.0
  LANGUAGES C CXX)

option(DLMS_BUILD_TESTS "Build GoogleTest tests" ON)
option(DLMS_BUILD_C_API "Build stable C ABI wrapper" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)

add_library(dlms_wrapper
  src/wrapper/wrapper_codec.cpp
  src/wrapper/wrapper_ports.cpp
  src/wrapper/wrapper_stream_decoder.cpp)

if(DLMS_BUILD_C_API)
  target_sources(dlms_wrapper
    PRIVATE
      src/wrapper/wrapper_c_api.cpp)
endif()

target_compile_features(dlms_wrapper PUBLIC cxx_std_11)

target_include_directories(dlms_wrapper
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(DLMS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
