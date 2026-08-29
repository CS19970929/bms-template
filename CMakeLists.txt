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
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wundef
        -Wformat=2
        -Wswitch-enum
        -Wcast-align
        -Wcast-qual
        -Wstrict-prototypes
        -Wmissing-prototypes
        -Wdouble-promotion
    )
endif()

add_library(bms_sanitizers INTERFACE)
if(BMS_ENABLE_SANITIZERS
   AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang"
   AND NOT CMAKE_CROSSCOMPILING)
    target_compile_options(bms_sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )
    target_link_options(bms_sanitizers INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -fno-sanitize-recover=all
    )
endif()

add_library(bms_base STATIC
    common/src/bms_crc32.c
    protocol/src/bms_frame.c
)
target_include_directories(bms_base PUBLIC
    common/include
    protocol/include
)
target_link_libraries(bms_base PRIVATE bms_warnings bms_sanitizers)

add_library(bms_boot_core STATIC
    bootloader/core/src/bms_boot_image.c
    bootloader/core/src/bms_boot_metadata.c
    bootloader/core/src/bms_boot_policy.c
    bootloader/core/src/bms_iap_session.c
    bootloader/core/src/bms_iap_service.c
)
target_include_directories(bms_boot_core PUBLIC
    bootloader/core/include
    protocol/include
    common/include
)
target_link_libraries(bms_boot_core
    PUBLIC bms_base
    PRIVATE bms_warnings bms_sanitizers
)

add_library(bms_app_core STATIC
    app/core/src/bms_protection.c
    app/core/src/bms_protection_manager.c
    app/core/src/bms_mos_policy.c
    app/core/src/bms_watchdog_supervisor.c
    app/core/src/bms_scheduler.c
    app/core/src/bms_state_machine.c
    app/core/src/bms_parameter.c
    app/core/src/bms_nvm_record.c
    app/core/src/bms_nvm_store.c
    app/core/src/bms_soc.c
)
target_include_directories(bms_app_core PUBLIC
    app/core/include
    common/include
    generated/parameters
)
target_link_libraries(bms_app_core
    PUBLIC bms_base
    PRIVATE bms_warnings bms_sanitizers
)

add_library(bms_afe_mock STATIC drivers/afe/src/bms_afe_mock.c)
target_include_directories(bms_afe_mock PUBLIC drivers/afe/include)
target_link_libraries(bms_afe_mock PRIVATE bms_warnings bms_sanitizers)

if(BMS_BUILD_HOST_TESTS)
    enable_testing()
    add_executable(bms_host_tests
        tests/host/test_main.c
        tests/host/test_app_core.c
    )
    target_link_libraries(bms_host_tests PRIVATE
        bms_base
        bms_boot_core
        bms_app_core
        bms_afe_mock
        bms_warnings
        bms_sanitizers
    )
    add_test(NAME bms_host_tests COMMAND bms_host_tests)

    add_executable(bms_protection_manager_atomic_test
        tests/host/test_protection_manager_atomic.c
    )
    target_link_libraries(bms_protection_manager_atomic_test PRIVATE
        bms_app_core
        bms_warnings
        bms_sanitizers
    )
    add_test(NAME bms_protection_manager_atomic_test COMMAND bms_protection_manager_atomic_test)

    add_executable(bms_nvm_power_loss_test
        tests/host/test_nvm_power_loss.c
    )
    target_link_libraries(bms_nvm_power_loss_test PRIVATE
        bms_app_core
        bms_warnings
        bms_sanitizers
    )
    add_test(NAME bms_nvm_power_loss_test COMMAND bms_nvm_power_loss_test)

    add_executable(bms_parameter_schema_test
        tests/host/test_parameter_schema.c
    )
    target_include_directories(bms_parameter_schema_test PRIVATE generated/parameters)
    target_link_libraries(bms_parameter_schema_test PRIVATE bms_warnings bms_sanitizers)
    add_test(NAME bms_parameter_schema_test COMMAND bms_parameter_schema_test)
endif()

if(CMAKE_CROSSCOMPILING)
    include(cmake/firmware.cmake)
endif()
