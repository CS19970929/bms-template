find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(BMS_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated/${BMS_TARGET})
file(MAKE_DIRECTORY ${BMS_GENERATED_DIR})
execute_process(
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/generate_target.py --target ${BMS_TARGET} --out ${BMS_GENERATED_DIR}
  RESULT_VARIABLE BMS_TARGET_GEN_RESULT)
if(NOT BMS_TARGET_GEN_RESULT EQUAL 0)
  message(FATAL_ERROR "target generation failed for ${BMS_TARGET}")
endif()

set(BMS_VENDOR_F0 ${CMAKE_SOURCE_DIR}/vendor/st/stm32f0_stdperiph_v1.5.0)

if(BMS_TARGET STREQUAL "stm32f030c8_mock")
  if(NOT EXISTS ${BMS_VENDOR_F0}/CMSIS/stm32f0xx.h)
    message(FATAL_ERROR "STM32F0 vendor files missing. Run: python tools/bootstrap_vendor.py")
  endif()
  enable_language(ASM)
  set(BMS_CPU_FLAGS -mcpu=cortex-m0 -mthumb)
  set(BMS_PLATFORM_INCLUDES ${CMAKE_SOURCE_DIR}/platform/stm32f0/include ${BMS_GENERATED_DIR} ${BMS_VENDOR_F0}/CMSIS ${BMS_VENDOR_F0}/StdPeriph/inc)
  foreach(core_target bms_base bms_boot_core bms_app_core)
    target_compile_options(${core_target} PRIVATE ${BMS_CPU_FLAGS} -ffunction-sections -fdata-sections)
  endforeach()

  add_library(bms_vendor_mcu STATIC
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_flash.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_gpio.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_iwdg.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_misc.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_rcc.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_syscfg.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_usart.c)
  target_include_directories(bms_vendor_mcu PUBLIC ${BMS_PLATFORM_INCLUDES})
  target_compile_definitions(bms_vendor_mcu PUBLIC STM32F030 USE_STDPERIPH_DRIVER)
  target_compile_options(bms_vendor_mcu PRIVATE ${BMS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections)

  add_library(bms_platform_mcu STATIC platform/stm32f0/src/system_stm32f030_clean.c platform/stm32f0/src/bms_platform_stm32f0.c)
  target_include_directories(bms_platform_mcu PUBLIC ${BMS_PLATFORM_INCLUDES} ${CMAKE_SOURCE_DIR}/bootloader/core/include)
  target_compile_definitions(bms_platform_mcu PUBLIC STM32F030 USE_STDPERIPH_DRIVER)
  target_compile_options(bms_platform_mcu PRIVATE ${BMS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections)
  target_link_libraries(bms_platform_mcu PUBLIC bms_vendor_mcu)

  function(add_bms_firmware name kind main_file linker image_define)
    add_executable(${name} platform/stm32f0/startup/startup_stm32f030_gcc.c ${main_file})
    set_target_properties(${name} PROPERTIES SUFFIX ".elf")
    target_include_directories(${name} PRIVATE ${BMS_PLATFORM_INCLUDES} ${CMAKE_SOURCE_DIR}/protocol/include ${CMAKE_SOURCE_DIR}/common/include ${CMAKE_SOURCE_DIR}/bootloader/core/include)
    target_compile_definitions(${name} PRIVATE STM32F030 USE_STDPERIPH_DRIVER ${image_define})
    target_compile_options(${name} PRIVATE ${BMS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections)
    target_link_options(${name} PRIVATE ${BMS_CPU_FLAGS} -T${linker} -Wl,--gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${name}.map --specs=nano.specs --specs=nosys.specs)
    if(kind STREQUAL "boot")
      target_link_libraries(${name} PRIVATE bms_base bms_boot_core bms_platform_mcu)
    else()
      target_link_libraries(${name} PRIVATE bms_base bms_app_core bms_platform_mcu)
    endif()
    add_custom_command(TARGET ${name} POST_BUILD
      COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${name}> ${CMAKE_CURRENT_BINARY_DIR}/${name}.hex
      COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${name}> ${CMAKE_CURRENT_BINARY_DIR}/${name}.bin
      COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${name}>)
  endfunction()

  add_bms_firmware(bms_bootloader boot bootloader/target/main.c ${BMS_GENERATED_DIR}/boot.ld BMS_BOOT_IMAGE)
  add_bms_firmware(bms_app app app/target/main.c ${BMS_GENERATED_DIR}/app.ld BMS_APP_IMAGE)

  file(GENERATE OUTPUT ${BMS_GENERATED_DIR}/keil_boot_sources.txt CONTENT
"$<JOIN:$<TARGET_PROPERTY:bms_base,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_boot_core,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_platform_mcu,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_vendor_mcu,SOURCES>,\n>\n${CMAKE_SOURCE_DIR}/bootloader/target/main.c\n")
  file(GENERATE OUTPUT ${BMS_GENERATED_DIR}/keil_app_sources.txt CONTENT
"$<JOIN:$<TARGET_PROPERTY:bms_base,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_app_core,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_platform_mcu,SOURCES>,\n>\n$<JOIN:$<TARGET_PROPERTY:bms_vendor_mcu,SOURCES>,\n>\n${CMAKE_SOURCE_DIR}/app/target/main.c\n")
elseif(BMS_TARGET STREQUAL "stm32f103c8_mock")
  message(FATAL_ERROR "STM32F103 target config is valid, but its hardware port is not declared complete yet.")
else()
  message(FATAL_ERROR "Unsupported BMS_TARGET=${BMS_TARGET}")
endif()
