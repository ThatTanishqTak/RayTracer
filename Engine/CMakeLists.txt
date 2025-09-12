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
        message(WARNING "glew32.dll not found; your app may fail at runtime if it's not on PATH.")
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
    # Try to locate a GLSL compiler. Building will still succeed if it is missing,
    # falling back to runtime compilation by the graphics API.
    find_program(ENGINE_GLSLANG_VALIDATOR glslangValidator)

    set(ENGINE_SHADER_DIR ${CMAKE_SOURCE_DIR}/Sandbox/Assets/Shaders)
    # Shaders follow <Name>.<stage>.glsl naming (e.g., RayTrace.comp.glsl)
    # so add an extra wildcard to capture the stage suffix.
    # Future improvement: enumerate shaders explicitly to catch typos early.
    file(GLOB ENGINE_GLSL_SHADERS ${ENGINE_SHADER_DIR}/*.*.glsl)

    foreach(ENGINE_SHADER ${ENGINE_GLSL_SHADERS})
        # Copy GLSL source to the output directory so it can be loaded at runtime.
        get_filename_component(ENGINE_SHADER_NAME ${ENGINE_SHADER} NAME)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Shaders
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ENGINE_SHADER}
                    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Shaders/${ENGINE_SHADER_NAME}
            COMMENT "Copying ${ENGINE_SHADER_NAME} to output directory")

        # If glslangValidator is available, emit SPIR-V binaries for future APIs.
        if(ENGINE_GLSLANG_VALIDATOR)
            get_filename_component(ENGINE_SHADER_BASENAME ${ENGINE_SHADER} NAME_WE)
            # Detect shader stage from filename (e.g., .vert.glsl, .frag.glsl, .comp.glsl).
            # Default to compute stage if no stage tag is present.
            string(REGEX MATCH "\.([^.]+)\.glsl$" _match "${ENGINE_SHADER_NAME}")
            set(ENGINE_SHADER_STAGE "${CMAKE_MATCH_1}")
            if(NOT ENGINE_SHADER_STAGE)
                set(ENGINE_SHADER_STAGE "comp")
            endif()

            set(ENGINE_SHADER_SPV ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Shaders/${ENGINE_SHADER_BASENAME}.spv)
            add_custom_command(OUTPUT ${ENGINE_SHADER_SPV}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Assets/Shaders
                COMMAND ${ENGINE_GLSLANG_VALIDATOR} -V -S ${ENGINE_SHADER_STAGE} ${ENGINE_SHADER} -o ${ENGINE_SHADER_SPV}
                DEPENDS ${ENGINE_SHADER}
                COMMENT "Compiling ${ENGINE_SHADER_BASENAME}.glsl (${ENGINE_SHADER_STAGE}) to SPIR-V")
            list(APPEND ENGINE_COMPILED_SHADERS ${ENGINE_SHADER_SPV})
        endif()
    endforeach()

    if(ENGINE_COMPILED_SHADERS)
        add_custom_target(EngineShaders ALL DEPENDS ${ENGINE_COMPILED_SHADERS})
        add_dependencies(${PROJECT_NAME} EngineShaders)
    endif()

    # Surface shader sources in the IDE for easy editing.
    if(ENGINE_GLSL_SHADERS)
        target_sources(${PROJECT_NAME} PRIVATE ${ENGINE_GLSL_SHADERS})
        source_group("Shaders" FILES ${ENGINE_GLSL_SHADERS})
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