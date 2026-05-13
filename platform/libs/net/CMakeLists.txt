cmake_minimum_required(VERSION 3.15)
project(net C)

SET (CMAKE_C_COMPILER_WORKS 1)
SET (CMAKE_CXX_COMPILER_WORKS 1)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../lib)

set(NET_VERSION_MAJOR    "1")
set(NET_VERSION_MINOR    "0")
set(NET_VERSION_REVISION "0")
set(NET_VERSION_STRING
    "${NET_VERSION_MAJOR}.${NET_VERSION_MINOR}.${NET_VERSION_REVISION}"
)

set(NET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(NET_INCLUDE_DIR ${NET_SOURCE_DIR}/include)

set(LWIP_INCLUDE_DIRS
    "${NET_INCLUDE_DIR}"
    "${NET_INCLUDE_DIR}/common"
    "${NET_INCLUDE_DIR}/os"
    "${NET_INCLUDE_DIR}/lwip"
    "${NET_INCLUDE_DIR}/lwip/ports"
)
set (LWIP_MBEDTLS_INCLUDE_DIRS ${NET_SOURCE_DIR}/modules/mbedtls/include)

# ========== 1. 交叉编译工具链 ==========
# 命令行:  cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-linux.cmake ..
# 也可把下面两行直接写死，但推荐用命令行方式：
# set(CMAKE_TOOLCHAIN_FILE "D:/users/hao_zhu/svn/1_pan/glass/app/libs/net/riscv.cmake")

# ========== 2. 模块开关 ==========
option(WITH_CJSON     "Build cjson module"     ON)
option(WITH_NOPOLL    "Build nopoll module"    ON)
option(WITH_WEBCLIENT "Build webclient module" ON)
# 后续再加模块时，只要再加一个 option 即可

# ========== 3. 收集 OBJECT 库 ==========
set(NETLIB_OBJS)          # 保存所有要合并的 OBJECT 库

add_subdirectory(modules/common)
list(APPEND NETLIB_OBJS $<TARGET_OBJECTS:common>)

add_subdirectory(modules/lwip)
list(APPEND NETLIB_OBJS $<TARGET_OBJECTS:lwipcontribports>)

add_subdirectory(modules/tinycrypt)
list(APPEND NETLIB_OBJS $<TARGET_OBJECTS:tinycrypt>)

if(WITH_CJSON)
    add_subdirectory(modules/cjson)
    list(APPEND NETLIB_OBJS $<TARGET_OBJECTS:cjson>)
endif()

if(WITH_NOPOLL)
    add_subdirectory(modules/nopoll)
    list(APPEND NETLIB_OBJS $<TARGET_OBJECTS:nopoll>)
endif()

if(WITH_WEBCLIENT)
    add_subdirectory(modules/webclient)
    list(APPEND NETLIB_OBJS $<TARGET_OBJECTS:webclient>)
endif()

add_subdirectory(modules/ugzip)
list(APPEND NETLIB_OBJS $<TARGET_OBJECTS:ugzip>)

set(ENABLE_PROGRAMS OFF)
set(ENABLE_TESTING OFF)
set(MBEDTLS_AS_SUBPROJECT OFF)
add_subdirectory(modules/mbedtls)

# ========== 4. 生成最终静态库 ==========
add_library(netz)
target_link_libraries(netz PUBLIC lwipcontribports lwipcore lwipallapps tinycrypt lwipmbedtls)
target_link_libraries(netz PUBLIC ugzip)

if(WITH_CJSON)
    target_link_libraries(netz PUBLIC cjson)
endif()

if(WITH_NOPOLL)
    target_link_libraries(netz PUBLIC nopoll)
endif()

if(WITH_WEBCLIENT)
    target_link_libraries(netz PUBLIC webclient)
endif()

target_link_libraries(netz PUBLIC
    MbedTLS::mbedtls
    MbedTLS::mbedcrypto
    MbedTLS::mbedx509
)

add_custom_command(TARGET netz
    POST_BUILD
    BYPRODUCTS ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/../libnet.a
    # COMMAND ${CMAKE_OBJCOPY}
    #         --rename-section .bss=.net_bss
    #         --rename-section .sbss=.net_sbss
    #         $<TARGET_FILE:netz>
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
            ${CMAKE_AR} -M < ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/libnet.mri
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/libnet.a ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/..
    COMMENT "Renaming sections and creating libnet.a from MRI script"
    VERBATIM
)
