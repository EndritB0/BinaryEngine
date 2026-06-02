include(FetchContent)

set(CMAKE_FOLDER "Vendor/SDL3")

set(SDL_SHARED OFF CACHE BOOL "Disable shared SDL3" FORCE)
set(SDL_STATIC ON CACHE BOOL "Enable static SDL3" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.10
)

FetchContent_MakeAvailable(SDL3)