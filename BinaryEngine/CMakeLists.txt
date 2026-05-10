add_library(BinaryEngine STATIC)

set(BINARYENGINE_SOURCES
    src/BinaryEngine/Application.cpp
    src/BinaryEngine/Application.h
    src/pch.h
)

set(BASE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set_target_properties(BinaryEngine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BASE_OUTPUT_DIR}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "" FILES ${BINARYENGINE_SOURCES})
source_group("Precompiled Headers" REGULAR_EXPRESSION ".*pch.*")

target_sources(BinaryEngine PRIVATE ${BINARYENGINE_SOURCES})

target_compile_features(BinaryEngine PRIVATE cxx_std_20)
target_include_directories(BinaryEngine PUBLIC src)
target_precompile_headers(BinaryEngine PRIVATE src/pch.h)

target_link_libraries(BinaryEngine PRIVATE SDL3::SDL3-static ProjectWarnings)
