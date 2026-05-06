if(DLMS_USE_SYSTEM_GTEST)
  find_package(GTest REQUIRED)
elseif(NOT TARGET GTest::gtest_main)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(dlms_wrapper_tests
  wrapper/test_wrapper_c_api.cpp
  wrapper/test_wrapper_c_header.c
  wrapper/test_wrapper_codec.cpp
  wrapper/test_wrapper_error.cpp
  wrapper/test_wrapper_ports.cpp
  wrapper/test_wrapper_stream_decoder.cpp
  wrapper/test_wrapper_vectors.cpp)

target_link_libraries(dlms_wrapper_tests
  PRIVATE
    dlms_wrapper
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(dlms_wrapper_tests)
