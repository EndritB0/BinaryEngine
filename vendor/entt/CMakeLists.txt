include(FetchContent)

set(CMAKE_FOLDER "Vendor/entt")

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.16.0
)

FetchContent_MakeAvailable(entt)