include(${LWIP_DIR}/contrib/ports/CMakeCommon.cmake)

include(${LWIP_DIR}/src/Filelists.cmake)

set(lwipcontribports_SRCS
    ${LWIP_CONTRIB_DIR}/ports/bnep_lwip.c
    ${LWIP_CONTRIB_DIR}/ports/rtthread/sys_arch.c
    ${LWIP_CONTRIB_DIR}/ports/rtthread/thread_gc.c
)
add_library(lwipcontribports OBJECT ${lwipcontribports_SRCS})
target_compile_options(lwipcontribports PRIVATE ${LWIP_COMPILER_FLAGS})
target_compile_definitions(lwipcontribports PRIVATE ${LWIP_DEFINITIONS} ${LWIP_MBEDTLS_DEFINITIONS})
target_include_directories(lwipcontribports PRIVATE ${LWIP_INCLUDE_DIRS} ${LWIP_MBEDTLS_INCLUDE_DIRS})
