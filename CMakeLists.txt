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
  src/sg200x_ll_csr.c
  src/sg200x_ll_dma.c
  src/sg200x_ll_i2c.c
  src/sg200x_ll_rcc.c
  src/sg200x_ll_spi.c
  src/sg200x_ll_uart.c
)
target_include_directories(sgll PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/inc"
)

target_compile_features(sgll PUBLIC c_std_23)
set_target_properties(sgll PROPERTIES C_EXTENSIONS OFF)

# RISC-V builds need the CSI core headers for both the library and consumers.
# A parent project may instead add this path with target_include_directories().
set(SGLL_CORE_INCLUDE_DIR "" CACHE PATH "SDK directory containing core_rv64.h")
if(SGLL_CORE_INCLUDE_DIR)
  target_include_directories(sgll SYSTEM PUBLIC "${SGLL_CORE_INCLUDE_DIR}")
endif()
