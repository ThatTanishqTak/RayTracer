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

if(MSVC AND ENGINE_ENABLE_GPU)
    # Locate the HLSL shader compilers. Prefer dxc when available.
    find_program(ENGINE_DXC dxc)
    find_program(ENGINE_FXC fxc)

    set(ENGINE_SHADER_DIR ${CMAKE_SOURCE_DIR}/Sandbox/Assets/Shaders)
    file(GLOB ENGINE_HLSL_SHADERS ${ENGINE_SHADER_DIR}/*.hlsl)

    foreach(ENGINE_SHADER ${ENGINE_HLSL_SHADERS})
        # Derive output name for the compiled shader object.
        get_filename_component(ENGINE_SHADER_NAME ${ENGINE_SHADER} NAME_WE)
        set(ENGINE_SHADER_OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Shaders/${ENGINE_SHADER_NAME}.cso)

        if(ENGINE_DXC)
            set(ENGINE_COMPILE_CMD ${ENGINE_DXC} /T cs_6_0 /E CSMain ${ENGINE_SHADER} /Fo ${ENGINE_SHADER_OUTPUT})
        elseif(ENGINE_FXC)
            set(ENGINE_COMPILE_CMD ${ENGINE_FXC} /T cs_5_0 /E CSMain ${ENGINE_SHADER} /Fo ${ENGINE_SHADER_OUTPUT})
        else()
            message(FATAL_ERROR "Neither dxc nor fxc shader compiler was found.")
        endif()

        # Compile the shader and mark the output as a build product.
        add_custom_command(OUTPUT ${ENGINE_SHADER_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Shaders
            COMMAND ${ENGINE_COMPILE_CMD}
            DEPENDS ${ENGINE_SHADER}
            COMMENT "Compiling ${ENGINE_SHADER_NAME}.hlsl"
            VERBATIM
        )

        list(APPEND ENGINE_COMPILED_SHADERS ${ENGINE_SHADER_OUTPUT})
    endforeach()

    if(ENGINE_COMPILED_SHADERS)
        # Ensure shaders are built alongside the engine and appear in Visual Studio.
        add_custom_target(EngineShaders ALL DEPENDS ${ENGINE_COMPILED_SHADERS})
        add_dependencies(${PROJECT_NAME} EngineShaders)
        target_sources(${PROJECT_NAME} PRIVATE ${ENGINE_HLSL_SHADERS})
        source_group("Shaders" FILES ${ENGINE_HLSL_SHADERS})
    endif()
endif()

if(ENGINE_ENABLE_GPU)
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