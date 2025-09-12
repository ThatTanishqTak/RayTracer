project(Engine LANGUAGES CXX)

# Option to enable GPU accelerated ray tracing. Disabled by default to keep
# builds portable on systems without compatible hardware or SDKs.
option(ENGINE_ENABLE_GPU "Enable GPU ray tracing support" ON)

# Raylib
add_subdirectory(vendor/raylib) # provides target: raylib

# ---- GLEW ------
set(GLEW_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glew)

# 1) Find headers
find_path(GLEW_INCLUDE_DIR
    NAMES GL/glew.h
    PATHS
      ${GLEW_ROOT}/include
      ${GLEW_ROOT}/inc
    NO_DEFAULT_PATH
)
if(NOT GLEW_INCLUDE_DIR)
    message(FATAL_ERROR "GLEW headers not found under ${GLEW_ROOT}. Expected GL/glew.h in include/.")
endif()

# 2) Find library (prefer static 'glew32s.lib', then 'glew32.lib')
set(_glew_lib_candidates
    ${GLEW_ROOT}/lib/${ARCH}
    ${GLEW_ROOT}/lib/Release/${ARCH}
    ${GLEW_ROOT}/lib/Debug/${ARCH}
    ${GLEW_ROOT}/lib/${CMAKE_BUILD_TYPE}/${ARCH}
    ${GLEW_ROOT}/lib
    ${GLEW_ROOT}/lib/Release
    ${GLEW_ROOT}/lib/Debug
)

unset(GLEW_LIB CACHE)
find_library(GLEW_LIB_STATIC NAMES glew32s
    PATHS ${_glew_lib_candidates} NO_DEFAULT_PATH)
find_library(GLEW_LIB_IMPORT NAMES glew32
    PATHS ${_glew_lib_candidates} NO_DEFAULT_PATH)

if(GLEW_LIB_STATIC)
    set(GLEW_LIB ${GLEW_LIB_STATIC})
    set(_glew_is_static TRUE)
elseif(GLEW_LIB_IMPORT)
    set(GLEW_LIB ${GLEW_LIB_IMPORT})
    set(_glew_is_static FALSE)
else()
    message(FATAL_ERROR
        "GLEW library not found. Looked for glew32s.lib or glew32.lib under:\n  ${_glew_lib_candidates}")
endif()

# 3) Declare imported target according to the variant
if(_glew_is_static)
    add_library(glew STATIC IMPORTED)
    set_target_properties(glew PROPERTIES
        IMPORTED_LOCATION "${GLEW_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}")
    target_compile_definitions(glew INTERFACE GLEW_STATIC)
else()
    add_library(glew SHARED IMPORTED)
    set_target_properties(glew PROPERTIES
        IMPORTED_IMPLIB "${GLEW_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}")
    # Try to locate the DLL to copy at runtime
    set(_glew_bin_candidates
        ${GLEW_ROOT}/bin/${ARCH}
        ${GLEW_ROOT}/bin
        ${GLEW_ROOT}/${ARCH}/bin
        ${GLEW_ROOT}
    )
    unset(GLEW_DLL CACHE)
    find_file(GLEW_DLL NAMES glew32.dll PATHS ${_glew_bin_candidates} NO_DEFAULT_PATH)
    if(GLEW_DLL)
        # Make the DLL show up next to your exe(s)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${GLEW_DLL}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/glew32.dll"
            COMMENT "Copying GLEW DLL to runtime output")
    else()
        message(WARNING "glew32.dll not found; your app may fail at runtime if it’s not on PATH.")
    endif()
endif()

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
    glew
)