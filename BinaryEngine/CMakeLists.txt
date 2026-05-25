add_library(BinaryEngine STATIC)

set(BINARYENGINE_SOURCES
    "src/pch.h"
    "src/BinaryEngine/vendor/stb_image.cpp"
    "src/BinaryEngine/BinaryEngine.h"
    "src/BinaryEngine/Application.cpp"
    "src/BinaryEngine/Application.h"
    "src/BinaryEngine/Core/Core.h"
    "src/BinaryEngine/Core/Math.h"
    "src/BinaryEngine/Core/Color.h"
    "src/BinaryEngine/Core/Timestep.h"
    "src/BinaryEngine/Core/UUID.cpp"
    "src/BinaryEngine/Core/UUID.h"
    "src/BinaryEngine/Core/Transform.h"
    "src/BinaryEngine/Window/Window.cpp"
    "src/BinaryEngine/Window/Window.h"
    "src/BinaryEngine/Window/WindowSpecification.h"
    "src/BinaryEngine/Renderer/Renderer.cpp"
    "src/BinaryEngine/Renderer/Renderer.h"
    "src/BinaryEngine/Renderer/Texture2D.cpp"
    "src/BinaryEngine/Renderer/Texture2D.h"
    "src/BinaryEngine/Renderer/OrthographicCamera.cpp"
    "src/BinaryEngine/Renderer/OrthographicCamera.h"
    "src/BinaryEngine/Log/Log.cpp"
    "src/BinaryEngine/Log/Log.h"
    "src/BinaryEngine/Event/MouseCodes.h"
    "src/BinaryEngine/Event/ScanCodes.h"
    "src/BinaryEngine/Event/ScanCodes.cpp"
    "src/BinaryEngine/Event/KeyCodes.h"
    "src/BinaryEngine/Event/KeyCodes.cpp"
    "src/BinaryEngine/Event/Event.h"
    "src/BinaryEngine/Event/EventTypes.h"
    "src/BinaryEngine/State/State.h"
    "src/BinaryEngine/State/Context.h"
    "src/BinaryEngine/State/StateManager.cpp"
    "src/BinaryEngine/State/StateManager.h"
    "src/BinaryEngine/Input/Input.cpp"
    "src/BinaryEngine/Input/Input.h"
    "src/BinaryEngine/Asset/AssetTypes.h"
    "src/BinaryEngine/Asset/Asset.h"
    "src/BinaryEngine/Asset/AssetManager.cpp"
    "src/BinaryEngine/Asset/AssetManager.h"
    "src/BinaryEngine/Scene/Components.h"
    "src/BinaryEngine/Scene/Entity.h"
    "src/BinaryEngine/Scene/Scene.h"
    "src/BinaryEngine/Scene/Scene.cpp"
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

target_link_libraries(BinaryEngine PUBLIC glm::glm spdlog::spdlog stb EnTT::EnTT PRIVATE SDL3::SDL3-static ProjectWarnings)
set_source_files_properties("src/BinaryEngine/vendor/stb_image.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
