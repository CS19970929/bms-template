set(BMS_VENDOR_F0 ${CMAKE_SOURCE_DIR}/vendor/st/stm32f0_stdperiph_v1.5.0)

if(BMS_TARGET STREQUAL "stm32f030c8_mock")
  if(NOT EXISTS ${BMS_VENDOR_F0}/CMSIS/stm32f0xx.h)
    message(FATAL_ERROR "STM32F0 vendor files missing. Run: python tools/bootstrap_vendor.py")
  endif()
  enable_language(ASM)
  set(BMS_CPU_FLAGS -mcpu=cortex-m0 -mthumb)
  set(BMS_PLATFORM_INCLUDES ${CMAKE_SOURCE_DIR}/platform/stm32f0/include ${BMS_VENDOR_F0}/CMSIS ${BMS_VENDOR_F0}/StdPeriph/inc)
  add_library(bms_vendor_mcu STATIC
    ${BMS_VENDOR_F0}/CMSIS/system_stm32f0xx.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_flash.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_gpio.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_iwdg.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_misc.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_rcc.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_syscfg.c
    ${BMS_VENDOR_F0}/StdPeriph/src/stm32f0xx_usart.c)
  target_include_directories(bms_vendor_mcu PUBLIC ${BMS_PLATFORM_INCLUDES})
  target_compile_definitions(bms_vendor_mcu PUBLIC STM32F030 USE_STDPERIPH_DRIVER)
  target_compile_options(bms_vendor_mcu PRIVATE ${BMS_CPU_FLAGS} -ffunction-sections -fdata-sections)

  add_library(bms_platform_mcu STATIC platform/stm32f0/src/bms_platform_stm32f0.c)
  target_include_directories(bms_platform_mcu PUBLIC ${BMS_PLATFORM_INCLUDES})
  target_compile_definitions(bms_platform_mcu PUBLIC STM32F030 USE_STDPERIPH_DRIVER)
  target_compile_options(bms_platform_mcu PRIVATE ${BMS_CPU_FLAGS} -ffunction-sections -fdata-sections)
  target_link_libraries(bms_platform_mcu PUBLIC bms_vendor_mcu)

  function(add_bms_firmware name main_file linker image_define)
    add_executable(${name}
      platform/stm32f0/startup/startup_stm32f030_gcc.c
      ${main_file})
    set_target_properties(${name} PROPERTIES SUFFIX ".elf")
    target_include_directories(${name} PRIVATE ${BMS_PLATFORM_INCLUDES})
    target_compile_definitions(${name} PRIVATE STM32F030 USE_STDPERIPH_DRIVER ${image_define})
    target_compile_options(${name} PRIVATE ${BMS_CPU_FLAGS} -O2 -ffunction-sections -fdata-sections)
    target_link_options(${name} PRIVATE ${BMS_CPU_FLAGS} -T${linker} -Wl,--gc-sections -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/${name}.map --specs=nano.specs --specs=nosys.specs)
    target_link_libraries(${name} PRIVATE bms_common bms_platform_mcu)
    add_custom_command(TARGET ${name} POST_BUILD
      COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${name}> ${CMAKE_CURRENT_BINARY_DIR}/${name}.hex
      COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${name}> ${CMAKE_CURRENT_BINARY_DIR}/${name}.bin
      COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${name}>)
  endfunction()

  add_bms_firmware(bms_bootloader bootloader/target/main.c ${CMAKE_SOURCE_DIR}/platform/stm32f0/linker/stm32f030c8_boot.ld BMS_BOOT_IMAGE)
  add_bms_firmware(bms_app app/target/main.c ${CMAKE_SOURCE_DIR}/platform/stm32f0/linker/stm32f030c8_app.ld BMS_APP_IMAGE)
elseif(BMS_TARGET STREQUAL "stm32f103c8_mock")
  message(FATAL_ERROR "STM32F103 port contract exists but vendor/startup integration is intentionally not declared complete yet.")
else()
  message(FATAL_ERROR "Unsupported BMS_TARGET=${BMS_TARGET}")
endif()
