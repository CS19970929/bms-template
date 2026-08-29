if(BMS_TARGET STREQUAL "stm32f030c8_mock")
  set(BMS_CPU_FLAGS -mcpu=cortex-m0 -mthumb)
  set(BMS_PLATFORM_DIR platform/stm32f0)
  set(BMS_LINKER_BOOT ${CMAKE_SOURCE_DIR}/platform/stm32f0/linker/stm32f030c8_boot.ld)
  set(BMS_LINKER_APP ${CMAKE_SOURCE_DIR}/platform/stm32f0/linker/stm32f030c8_app.ld)
elseif(BMS_TARGET STREQUAL "stm32f103c8_mock")
  set(BMS_CPU_FLAGS -mcpu=cortex-m3 -mthumb)
  set(BMS_PLATFORM_DIR platform/stm32f1)
  set(BMS_LINKER_BOOT ${CMAKE_SOURCE_DIR}/platform/stm32f1/linker/stm32f103c8_boot.ld)
  set(BMS_LINKER_APP ${CMAKE_SOURCE_DIR}/platform/stm32f1/linker/stm32f103c8_app.ld)
else()
  message(FATAL_ERROR "Unsupported BMS_TARGET=${BMS_TARGET}")
endif()

# Platform startup/vendor libraries are intentionally separate from production core.
# The first hardware port is enabled only after the exact ST StdPeriph/CMSIS package is vendored.
message(STATUS "Configured firmware target ${BMS_TARGET}; hardware port source enablement follows vendor import.")
