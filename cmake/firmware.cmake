find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(BMS_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated/${BMS_TARGET}")
file(MAKE_DIRECTORY "${BMS_GENERATED_DIR}")

execute_process(
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/generate_target.py"
            --target "${BMS_TARGET}"
            --out "${BMS_GENERATED_DIR}"
    RESULT_VARIABLE BMS_TARGET_GEN_RESULT
)
if(NOT BMS_TARGET_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "target generation failed for ${BMS_TARGET}")
endif()

include("${BMS_GENERATED_DIR}/target.cmake")

if(BMS_GENERATED_MCU_CORE STREQUAL "cortex-m0")
    set(BMS_CPU_FLAGS -mcpu=cortex-m0 -mthumb)
elseif(BMS_GENERATED_MCU_CORE STREQUAL "cortex-m3")
    set(BMS_CPU_FLAGS -mcpu=cortex-m3 -mthumb)
else()
    message(FATAL_ERROR "Unsupported MCU core: ${BMS_GENERATED_MCU_CORE}")
endif()

function(bms_apply_core_cpu)
    foreach(t bms_base bms_boot_core bms_app_core)
        target_compile_options(${t} PRIVATE ${BMS_CPU_FLAGS} -ffunction-sections -fdata-sections)
    endforeach()
endfunction()

function(bms_add_image name kind main_file startup linker define)
    add_executable(${name} ${startup} ${main_file})
    set_target_properties(${name} PROPERTIES SUFFIX ".elf")

    target_include_directories(${name} PRIVATE
        ${BMS_MCU_INCLUDES}
        "${CMAKE_SOURCE_DIR}/platform/common/include"
        "${CMAKE_SOURCE_DIR}/protocol/include"
        "${CMAKE_SOURCE_DIR}/common/include"
        "${CMAKE_SOURCE_DIR}/bootloader/core/include"
        "${CMAKE_SOURCE_DIR}/app/core/include"
    )
    target_compile_definitions(${name} PRIVATE ${define})
    target_compile_options(${name} PRIVATE
        ${BMS_CPU_FLAGS}
        -O2
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(${name} PRIVATE
        ${BMS_CPU_FLAGS}
        -T${linker}
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${name}.map
        --specs=nano.specs
        --specs=nosys.specs
    )

    if(kind STREQUAL "boot")
        target_link_libraries(${name} PRIVATE bms_base bms_boot_core bms_platform_mcu bms_warnings)
    elseif(kind STREQUAL "app")
        target_link_libraries(${name} PRIVATE bms_base bms_app_core bms_platform_mcu bms_warnings)
    else()
        message(FATAL_ERROR "unknown firmware image kind: ${kind}")
    endif()

    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex
                $<TARGET_FILE:${name}> "${CMAKE_CURRENT_BINARY_DIR}/${name}.hex"
        COMMAND ${CMAKE_OBJCOPY} -O binary
                $<TARGET_FILE:${name}> "${CMAKE_CURRENT_BINARY_DIR}/${name}.bin"
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${name}>
        VERBATIM
    )
endfunction()

function(bms_generate_keil_source_manifests)
    file(GENERATE
        OUTPUT "${BMS_GENERATED_DIR}/keil_boot_sources.txt"
        CONTENT "$<JOIN:$<TARGET_PROPERTY:bms_base,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_boot_core,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_platform_mcu,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_vendor_mcu,SOURCES>,\n>\n${CMAKE_SOURCE_DIR}/bootloader/target/main.c\n"
    )
    file(GENERATE
        OUTPUT "${BMS_GENERATED_DIR}/keil_app_sources.txt"
        CONTENT "$<JOIN:$<TARGET_PROPERTY:bms_base,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_app_core,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_platform_mcu,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_vendor_mcu,SOURCES>,\n>\n${CMAKE_SOURCE_DIR}/app/target/main.c\n"
    )
endfunction()

bms_apply_core_cpu()

if(BMS_GENERATED_MCU_FAMILY STREQUAL "stm32f0")
    if(NOT BMS_GENERATED_MCU_VENDOR STREQUAL "stm32f0_stdperiph_v1.5.0")
        message(FATAL_ERROR "Unexpected STM32F0 vendor profile: ${BMS_GENERATED_MCU_VENDOR}")
    endif()
    set(V "${CMAKE_SOURCE_DIR}/vendor/st/${BMS_GENERATED_MCU_VENDOR}")
    if(NOT EXISTS "${V}/CMSIS/stm32f0xx.h")
        message(FATAL_ERROR "Run: python tools/bootstrap_vendor.py --family f0")
    endif()

    set(BMS_MCU_INCLUDES
        "${CMAKE_SOURCE_DIR}/platform/stm32f0/include"
        "${BMS_GENERATED_DIR}"
        "${V}/CMSIS"
        "${V}/StdPeriph/inc"
    )

    add_library(bms_vendor_mcu STATIC
        "${V}/StdPeriph/src/stm32f0xx_flash.c"
        "${V}/StdPeriph/src/stm32f0xx_gpio.c"
        "${V}/StdPeriph/src/stm32f0xx_iwdg.c"
        "${V}/StdPeriph/src/stm32f0xx_misc.c"
        "${V}/StdPeriph/src/stm32f0xx_rcc.c"
        "${V}/StdPeriph/src/stm32f0xx_syscfg.c"
        "${V}/StdPeriph/src/stm32f0xx_usart.c"
    )
    target_include_directories(bms_vendor_mcu PUBLIC ${BMS_MCU_INCLUDES})
    target_compile_definitions(bms_vendor_mcu PUBLIC ${BMS_GENERATED_MCU_DEFINE} USE_STDPERIPH_DRIVER)
    target_compile_options(bms_vendor_mcu PRIVATE
        ${BMS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections
    )

    add_library(bms_platform_mcu STATIC
        platform/stm32f0/src/system_stm32f030_clean.c
        platform/stm32f0/src/bms_platform_stm32f0.c
    )
    target_include_directories(bms_platform_mcu PUBLIC
        ${BMS_MCU_INCLUDES}
        "${CMAKE_SOURCE_DIR}/platform/common/include"
        "${CMAKE_SOURCE_DIR}/bootloader/core/include"
    )
    target_compile_definitions(bms_platform_mcu PUBLIC ${BMS_GENERATED_MCU_DEFINE} USE_STDPERIPH_DRIVER)
    target_compile_options(bms_platform_mcu PRIVATE
        ${BMS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections
    )
    target_link_libraries(bms_platform_mcu PUBLIC bms_vendor_mcu PRIVATE bms_warnings)
    set(BMS_STARTUP "${CMAKE_SOURCE_DIR}/platform/stm32f0/startup/startup_stm32f030_gcc.c")

elseif(BMS_GENERATED_MCU_FAMILY STREQUAL "stm32f1")
    if(NOT BMS_GENERATED_MCU_VENDOR STREQUAL "stm32f10x_stdperiph_v3.5.0")
        message(FATAL_ERROR "Unexpected STM32F1 vendor profile: ${BMS_GENERATED_MCU_VENDOR}")
    endif()
    set(V "${CMAKE_SOURCE_DIR}/vendor/st/${BMS_GENERATED_MCU_VENDOR}")
    if(NOT EXISTS "${V}/CMSIS/stm32f10x.h")
        message(FATAL_ERROR "Run: python tools/bootstrap_vendor.py --family f1")
    endif()

    set(BMS_MCU_INCLUDES
        "${CMAKE_SOURCE_DIR}/platform/stm32f1/include"
        "${BMS_GENERATED_DIR}"
        "${V}/CMSIS"
        "${V}/StdPeriph/inc"
    )

    add_library(bms_vendor_mcu STATIC
        "${V}/StdPeriph/src/stm32f10x_flash.c"
        "${V}/StdPeriph/src/stm32f10x_gpio.c"
        "${V}/StdPeriph/src/stm32f10x_iwdg.c"
        "${V}/StdPeriph/src/misc.c"
        "${V}/StdPeriph/src/stm32f10x_rcc.c"
        "${V}/StdPeriph/src/stm32f10x_usart.c"
    )
    target_include_directories(bms_vendor_mcu PUBLIC ${BMS_MCU_INCLUDES})
    target_compile_definitions(bms_vendor_mcu PUBLIC ${BMS_GENERATED_MCU_DEFINE} USE_STDPERIPH_DRIVER)
    target_compile_options(bms_vendor_mcu PRIVATE
        ${BMS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections
    )

    add_library(bms_platform_mcu STATIC
        platform/stm32f1/src/system_stm32f103_clean.c
        platform/stm32f1/src/bms_platform_stm32f1.c
    )
    target_include_directories(bms_platform_mcu PUBLIC
        ${BMS_MCU_INCLUDES}
        "${CMAKE_SOURCE_DIR}/platform/common/include"
        "${CMAKE_SOURCE_DIR}/bootloader/core/include"
    )
    target_compile_definitions(bms_platform_mcu PUBLIC ${BMS_GENERATED_MCU_DEFINE} USE_STDPERIPH_DRIVER)
    target_compile_options(bms_platform_mcu PRIVATE
        ${BMS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections
    )
    target_link_libraries(bms_platform_mcu PUBLIC bms_vendor_mcu PRIVATE bms_warnings)
    set(BMS_STARTUP "${CMAKE_SOURCE_DIR}/platform/stm32f1/startup/startup_stm32f103_gcc.c")

else()
    message(FATAL_ERROR "Unsupported MCU family: ${BMS_GENERATED_MCU_FAMILY}")
endif()

bms_add_image(
    bms_bootloader
    boot
    "${CMAKE_SOURCE_DIR}/bootloader/target/main.c"
    "${BMS_STARTUP}"
    "${BMS_GENERATED_DIR}/boot.ld"
    BMS_BOOT_IMAGE
)
bms_add_image(
    bms_app
    app
    "${CMAKE_SOURCE_DIR}/app/target/main.c"
    "${BMS_STARTUP}"
    "${BMS_GENERATED_DIR}/app.ld"
    BMS_APP_IMAGE
)

bms_generate_keil_source_manifests()
