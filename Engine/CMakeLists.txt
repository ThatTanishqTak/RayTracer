project(Engine LANGUAGES CXX)

# Option to enable GPU accelerated ray tracing. Disabled by default to keep
# builds portable on systems without compatible hardware or SDKs.
option(ENGINE_ENABLE_GPU "Enable GPU ray tracing support" ON)

# Raylib
add_subdirectory(vendor/raylib) # provides target: raylib

# ---- ImGui (core) ----
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui)
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    # Optional but handy for testing:
    ${IMGUI_DIR}/imgui_demo.cpp
)
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
)

# ---- rlImGui backend ----
set(RLIMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rlImGui)
add_library(rlimgui STATIC
    ${RLIMGUI_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui PUBLIC
    ${RLIMGUI_DIR}
    ${IMGUI_DIR}
)
target_link_libraries(rlimgui PUBLIC imgui raylib)

# ---- stb -------

# ---- Engine sources ----
file(GLOB_RECURSE ENGINE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${PROJECT_NAME} STATIC ${ENGINE_SOURCE})

if(ENGINE_ENABLE_GPU)
    # Directory containing shader source files
    set(ENGINE_SHADER_DIR ${CMAKE_SOURCE_DIR}/Sandbox/Assets/Shaders)

    # Gather vertex and fragment shaders for the build
    file(GLOB ENGINE_SHADER_FILES
        ${ENGINE_SHADER_DIR}/*.vert
        ${ENGINE_SHADER_DIR}/*.frag
    )

    foreach(ENGINE_SHADER ${ENGINE_SHADER_FILES})
        # Destination path inside the runtime output directory
        get_filename_component(ENGINE_SHADER_NAME ${ENGINE_SHADER} NAME)
        set(ENGINE_SHADER_OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Shaders/${ENGINE_SHADER_NAME})

        # Copy the shader file, creating the directory if necessary
        add_custom_command(OUTPUT ${ENGINE_SHADER_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Shaders
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ENGINE_SHADER} ${ENGINE_SHADER_OUTPUT}
            DEPENDS ${ENGINE_SHADER}
            COMMENT "Copying ${ENGINE_SHADER_NAME}"
            VERBATIM
        )

        list(APPEND ENGINE_COPIED_SHADERS ${ENGINE_SHADER_OUTPUT})
    endforeach()

    if(ENGINE_COPIED_SHADERS)
        # Copy shaders alongside the engine and expose them in Visual Studio for editing
        add_custom_target(EngineShaders ALL DEPENDS ${ENGINE_COPIED_SHADERS})
        add_dependencies(${PROJECT_NAME} EngineShaders)
        target_sources(${PROJECT_NAME} PRIVATE ${ENGINE_SHADER_FILES})
        source_group("Shaders" FILES ${ENGINE_SHADER_FILES})
    endif()

    target_compile_definitions(${PROJECT_NAME} PUBLIC ENGINE_ENABLE_GPU)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src>
    $<BUILD_INTERFACE:${IMGUI_DIR}>
    $<BUILD_INTERFACE:${RLIMGUI_DIR}>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    rlimgui
)