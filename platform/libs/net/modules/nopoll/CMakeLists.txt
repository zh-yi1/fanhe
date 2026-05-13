set(nopoll_src 
    nopoll_conn_opts.c
    nopoll_conn.c
    nopoll_ctx.c
    nopoll_decl.c
    nopoll_io.c
    nopoll_listener.c
    nopoll_log.c
    nopoll_loop.c
    nopoll_msg.c
    nopoll_rtthread.c
    nopoll.c
)

add_library(nopoll OBJECT
    ${nopoll_src}
)
target_compile_options(nopoll PRIVATE ${NET_COMPILER_FLAGS})
target_include_directories(nopoll PRIVATE
    ${LWIP_INCLUDE_DIRS}
    ${NET_INCLUDE_DIR}/tinycrypt
    ${NET_INCLUDE_DIR}/nopoll
)

add_library(nopoll_example OBJECT
    nopoll_client.c
)
target_compile_options(nopoll_example PRIVATE ${NET_COMPILER_FLAGS})
target_include_directories(nopoll_example PRIVATE
    ${LWIP_INCLUDE_DIRS}
    ${NET_INCLUDE_DIR}/tinycrypt
    ${NET_INCLUDE_DIR}/nopoll
)
