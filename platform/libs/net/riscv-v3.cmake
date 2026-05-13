set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          riscv)

# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

set(BAREMETAL_RISCV_TOOLCHAIN_PATH  "C:/Program Files (x86)/RV32-Toolchain/RV32-V3/bin/")

set(CMAKE_AR                        ${BAREMETAL_RISCV_TOOLCHAIN_PATH}riscv32-elf-ar.exe)
set(CMAKE_ASM_COMPILER              ${BAREMETAL_RISCV_TOOLCHAIN_PATH}riscv32-elf-gcc.exe)
set(CMAKE_C_COMPILER                ${BAREMETAL_RISCV_TOOLCHAIN_PATH}riscv32-elf-gcc.exe)
set(CMAKE_CXX_COMPILER              ${BAREMETAL_RISCV_TOOLCHAIN_PATH}riscv32-elf-gcc.exe)
set(CMAKE_LINKER                    ${BAREMETAL_RISCV_TOOLCHAIN_PATH}riscv32-elf-ld.exe)
set(CMAKE_OBJCOPY                   ${BAREMETAL_RISCV_TOOLCHAIN_PATH}riscv32-elf-objcopy.exe CACHE INTERNAL "")

SET (CMAKE_C_COMPILER_WORKS 1)
SET (CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(NET_COMPILER_FLAGS
-Os
-Wall
-Wno-address-of-packed-member
-ffunction-sections
-fdata-sections
)

set(CMAKE_C_FLAGS "-march=rv32imc_zba_zbb_zbc_zbs_zca_zcb_zcmp_zfinx_xbs1 -mjump-tables-in-text --param=min-pagesize=0")
