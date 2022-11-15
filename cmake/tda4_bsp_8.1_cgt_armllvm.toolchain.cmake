set(SOC_ID SOC_J721E)
set(SOC_ID_OUT J7)

include(${CMAKE_CURRENT_LIST_DIR}/tda4_bsp_8_cgt_armllvm.cmake)

list(APPEND CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
    ${BSP_TOPDIR}/pdk_jacinto_08_01_00_36/packages
)
