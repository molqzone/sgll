# sgll - SG200X stateless LL-style peripheral library.
#
# Consumers add this directory via add_subdirectory() and link PUBLIC:
#
#   add_subdirectory("${SG200X_BSP_ROOT}/sgll" sgll)
#   target_link_libraries(<target> PUBLIC sgll)
#
# Register primitives stay inline in inc/. Compound operations and shared
# lookup data are compiled once from src/. Headers and sources require C23.

cmake_minimum_required(VERSION 3.22)

project(sgll LANGUAGES C)

add_library(sgll STATIC
  src/sg200x_ll_adc.c
  src/sg200x_ll_csr.c
  src/sg200x_ll_dma.c
  src/sg200x_ll_i2c.c
  src/sg200x_ll_mbox.c
  src/sg200x_ll_pwm.c
  src/sg200x_ll_rcc.c
  src/sg200x_ll_spi.c
  src/sg200x_ll_uart.c
  src/sg200x_ll_wdt.c
)
target_include_directories(sgll PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc"
  "${CMAKE_CURRENT_SOURCE_DIR}/device"
)

target_compile_features(sgll PUBLIC c_std_23)
set_target_properties(sgll PROPERTIES C_EXTENSIONS OFF)

# The core layer is self-contained: RISC-V builds use the inline assembly in
# inc/sg200x_ll_csr.h and need no SDK header. Set SGLL_CORE_BACKEND_SDK to compile
# the same primitives against the vendor core_rv64.h/csi_rv64_gcc.h reference
# backend instead; SGLL_CORE_INCLUDE_DIR then points at the SDK arch directory
# holding it. Both backends expose identical primitives, so the switch is an A/B
# build knob.
option(SGLL_CORE_BACKEND_SDK "Compile core primitives against the SDK core_rv64.h backend" OFF)
set(SGLL_CORE_INCLUDE_DIR "" CACHE PATH "SDK arch directory containing core_rv64.h (SDK backend only)")
if(SGLL_CORE_BACKEND_SDK)
  if(NOT SGLL_CORE_INCLUDE_DIR)
    message(FATAL_ERROR "SGLL_CORE_BACKEND_SDK requires SGLL_CORE_INCLUDE_DIR")
  endif()
  target_include_directories(sgll SYSTEM PUBLIC "${SGLL_CORE_INCLUDE_DIR}")
  target_compile_definitions(sgll PUBLIC SGLL_CORE_BACKEND_SDK)
endif()
