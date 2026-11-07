include(FetchContent)

set(CMAKE_FOLDER "Vendor/box2d")

FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG v3.2.0
)

FetchContent_MakeAvailable(box2d)