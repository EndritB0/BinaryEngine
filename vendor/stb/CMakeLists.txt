include(FetchContent)

set(CMAKE_FOLDER "Vendor/stb")

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)

FetchContent_MakeAvailable(stb)

add_library(stb STATIC
    stb_image.cpp
    stb_truetype.cpp
    stb_vorbis.cpp
)
target_include_directories(stb PUBLIC ${stb_SOURCE_DIR})