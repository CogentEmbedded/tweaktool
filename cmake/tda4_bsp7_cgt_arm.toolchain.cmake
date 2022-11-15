set(CMAKE_SYSTEM_NAME Concerto)
set(CMAKE_SYSTEM_PROCESSOR TI_ARM_R5F)

set(BSP_TOPDIR "$ENV{PSDKR_PATH}")

message(STATUS "PSDK: ${BSP_TOPDIR}")

set(SOC_ID_OUT J7)

set(CGT_ROOT
    "${BSP_TOPDIR}/ti-cgt-arm_20.2.0.LTS"
    CACHE PATH "TI CGT Toolchain path")

set(R5F_OS "SYSBIOS")

set(CMAKE_AR
    "${CGT_ROOT}/bin/armar"
    CACHE PATH "TI CGT ar Program")
set(CMAKE_NM
    "${CGT_ROOT}/bin/armnm"
    CACHE PATH "TI CGT nm Program")
set(CMAKE_OBJCOPY
    "${CGT_ROOT}/bin/armobjcopy"
    CACHE PATH "TI CGT objcopy Program")
set(CMAKE_OBJDUMP
    "${CGT_ROOT}/bin/armobjdump"
    CACHE PATH "TI CGT objdump Program")
set(CMAKE_LINKER
    "${CGT_ROOT}/bin/armlink"
    CACHE PATH "TI CGT Linker Program")
set(CMAKE_STRIP
    "${CGT_ROOT}/bin/armstrip"
    CACHE PATH "TI CGT Strip Program")

set(CMAKE_C_COMPILER "${CGT_ROOT}/bin/armcl")
set(CMAKE_CXX_COMPILER "${CMAKE_C_COMPILER}")

# Embedded firmware does not have any rootfs as such, so emulate it as well as possible
set(CMAKE_FIND_ROOT_PATH "${CGT_ROOT}")
set(CMAKE_PREFIX_PATH "${BSP_TOPDIR}/.sysbios-cmake-rootfs/")

# search headers and libraries in the target environment, search programs in the
# host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# SYSBIOS minimal include configuration to be able to compile a test program
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
    ${BSP_TOPDIR}/bios_6_83_00_18/packages
    ${BSP_TOPDIR}/ti-cgt-arm_20.2.0.LTS/include
    ${BSP_TOPDIR}/xdctools_3_61_03_29_core/packages
    ${BSP_TOPDIR}/pdk_jacinto_07_01_00_45/packages
    ${BSP_TOPDIR}/tiovx/include
    ${BSP_TOPDIR}/tiovx/kernels/include
    ${BSP_TOPDIR}/vision_apps
    ${BSP_TOPDIR}/vision_apps/apps/basic_demos/app_tirtos)

set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_STANDARD_INCLUDE_DIRECTORIES})

set(CMAKE_C_FLAGS
    "-DSOC_J721E \
     -DSYSBIOS \
     -DJ7 \
     -DR5F=R5F \
     -DTARGET_NUM_CORES=1 \
     -DTARGET_ARCH=32 \
     -DARCH_32 \
     -DARM \
     -DTARGET_BUILD=1 \
     -Dxdc_target_name__=ti/targets/arm/elf/R5F \
     -Dxdc_target_types__=ti/targets/arm/elf/std.h \
     --endian=little \
     --abi=eabi \
     -mv=7R5 \
     --float_support=vfpv3d16 \
     --emit_warnings_as_errors \
     --c99")

set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})

set(CMAKE_C_FLAGS_RELEASE_INIT "--symdebug:none -DNDEBUG -O3 --gen_opt_info=2")
set(CMAKE_CXX_FLAGS_RELEASE_INIT ${CMAKE_C_FLAGS_RELEASE_INIT})

set(CMAKE_LINKER_FLAGS "-z --reread_libs --rom_model --search_path=${CGT_ROOT}/lib")
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_LINKER_FLAGS})

set(CMAKE_C_STANDARD_LIBRARIES "--library=rtsv7R4_T_le_v3D16_eabi.lib")
set(CMAKE_CXX_STANDARD_LIBRARIES ${CMAKE_C_STANDARD_LIBRARIES})

set(CMAKE_LINK_LIBRARY_FLAG --library=)
set(CMAKE_LIBRARY_PATH_FLAG --search-path=)

set(CMAKE_INSTALL_INCLUDEDIR include)
set(CMAKE_INSTALL_BINDIR bin)
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE)
set(CMAKE_INSTALL_LIBDIR lib/${SOC_ID_OUT}/R5F/${R5F_OS}/${BUILD_TYPE})
