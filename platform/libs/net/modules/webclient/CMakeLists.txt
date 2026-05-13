add_library(webclient OBJECT
    src/webclient.c
)
target_compile_options(webclient PRIVATE ${NET_COMPILER_FLAGS})
target_include_directories(webclient PRIVATE
    ${LWIP_INCLUDE_DIRS}
    ${NET_INCLUDE_DIR}/webclient
)
