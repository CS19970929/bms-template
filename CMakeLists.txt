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

add_library(bms_base STATIC common/src/bms_crc32.c protocol/src/bms_frame.c)
target_include_directories(bms_base PUBLIC common/include protocol/include)
target_link_libraries(bms_base PRIVATE bms_warnings bms_sanitizers)

add_library(bms_boot_core STATIC
  bootloader/core/src/bms_boot_image.c
  bootloader/core/src/bms_boot_metadata.c
  bootloader/core/src/bms_boot_policy.c
  bootloader/core/src/bms_iap_session.c
  bootloader/core/src/bms_iap_service.c)
target_include_directories(bms_boot_core PUBLIC bootloader/core/include protocol/include common/include)
target_link_libraries(bms_boot_core PUBLIC bms_base PRIVATE bms_warnings bms_sanitizers)

add_library(bms_app_core STATIC app/core/src/bms_protection.c)
target_include_directories(bms_app_core PUBLIC app/core/include)
target_link_libraries(bms_app_core PRIVATE bms_warnings bms_sanitizers)

add_library(bms_afe_mock STATIC drivers/afe/src/bms_afe_mock.c)
target_include_directories(bms_afe_mock PUBLIC drivers/afe/include)
target_link_libraries(bms_afe_mock PRIVATE bms_warnings bms_sanitizers)

if(BMS_BUILD_HOST_TESTS)
  enable_testing()
  add_executable(bms_host_tests tests/host/test_main.c)
  target_link_libraries(bms_host_tests PRIVATE bms_base bms_boot_core bms_app_core bms_afe_mock bms_warnings bms_sanitizers)
  add_test(NAME bms_host_tests COMMAND bms_host_tests)
endif()

if(CMAKE_CROSSCOMPILING)
  include(cmake/firmware.cmake)
endif()
