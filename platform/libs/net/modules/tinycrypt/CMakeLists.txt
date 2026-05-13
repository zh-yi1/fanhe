add_library(tinycrypt OBJECT
    src/tiny_aes.c
    src/tiny_base64.c
    src/tiny_md5.c
    src/tiny_sha1.c
    src/tiny_sha2.c
)
target_compile_options(tinycrypt PRIVATE ${NET_COMPILER_FLAGS})
target_include_directories(tinycrypt PRIVATE
    ${NET_INCLUDE_DIR}/tinycrypt
)
