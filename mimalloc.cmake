include(FetchContent)
FetchContent_Declare(
    MIMALLOC
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG dev3
)
FetchContent_MakeAvailable(MIMALLOC)
FetchContent_GetProperties(MIMALLOC
    SOURCE_DIR MIMALLOC_SOURCE_DIR
    BINARY_DIR MIMALLOC_BINARY_DIR
)

add_definitions("-DADT_USE_MIMALLOC")
