# sgll - SG200X stateless LL-style peripheral library (header-only).
#
# Consumers add this directory via add_subdirectory() and link PUBLIC:
#
#   add_subdirectory("${SG200X_BSP_ROOT}/sgll" sgll)
#   target_link_libraries(<target> PUBLIC sgll)
#
# There are no sources to compile; the target only exports the include
# path.  Language requirement is deliberately C11 (see PLAN section 3.4):
# the headers use no C23-only syntax, so any toolchain works.

cmake_minimum_required(VERSION 3.22)

project(sgll LANGUAGES C)

add_library(sgll INTERFACE)
target_include_directories(sgll INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/inc")

# The library itself has no compile requirement beyond C11 (_Static_assert
# and static inline); consumers keep their own standard settings.
target_compile_features(sgll INTERFACE c_std_11)
