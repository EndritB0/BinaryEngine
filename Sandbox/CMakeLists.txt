add_executable(Sandbox)

set(SANDBOX_SOURCES
    src/Sandbox/main.cpp
    src/pch.h
)

set(BASE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set_target_properties(Sandbox PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BASE_OUTPUT_DIR}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES ${SANDBOX_SOURCES})
source_group("Precompiled Headers" REGULAR_EXPRESSION ".*pch.*")

target_sources(Sandbox PRIVATE ${SANDBOX_SOURCES})

target_compile_features(Sandbox PRIVATE cxx_std_20)
target_include_directories(Sandbox PRIVATE src)
target_precompile_headers(Sandbox PRIVATE src/pch.h)

target_link_libraries(Sandbox PRIVATE BinaryEngine ProjectWarnings)