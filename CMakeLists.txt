cmake_minimum_required(VERSION 3.24)
project(bms_template LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(BMS_BUILD_HOST_TESTS "Build host-side production-core tests" ON)
option(BMS_ENABLE_SANITIZERS "Enable ASan/UBSan for host builds" OFF)
set(BMS_TARGET "host_mock" CACHE STRING "Target profile")

add_library(bms_warnings INTERFACE)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(bms_warnings INTERFACE
    -Wall -Wextra -Werror -Wconversion -Wsign-conversion -Wshadow -Wundef
    -Wformat=2 -Wswitch-enum -Wcast-align -Wcast-qual -Wstrict-prototypes
    -Wmissing-prototypes -Wdouble-promotion)
endif()

if(BMS_ENABLE_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT CMAKE_CROSSCOMPILING)
  add_library(bms_sanitizers INTERFACE)
  target_compile_options(bms_sanitizers INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
  target_link_options(bms_sanitizers INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
else()
  add_library(bms_sanitizers INTERFACE)
endif()

add_library(bms_common STATIC
  common/src/bms_crc32.c
  protocol/src/bms_frame.c
  bootloader/core/src/bms_boot_image.c
  bootloader/core/src/bms_boot_metadata.c
  bootloader/core/src/bms_boot_policy.c
  bootloader/core/src/bms_iap_session.c
  app/core/src/bms_protection.c
  drivers/afe/src/bms_afe_mock.c)

target_include_directories(bms_common PUBLIC
  common/include protocol/include bootloader/core/include app/core/include drivers/afe/include)
target_link_libraries(bms_common PRIVATE bms_warnings bms_sanitizers)

if(BMS_BUILD_HOST_TESTS)
  enable_testing()
  add_executable(bms_host_tests tests/host/test_main.c)
  target_link_libraries(bms_host_tests PRIVATE bms_common bms_warnings bms_sanitizers)
  add_test(NAME bms_host_tests COMMAND bms_host_tests)
endif()

if(CMAKE_CROSSCOMPILING)
  include(cmake/firmware.cmake)
endif()
