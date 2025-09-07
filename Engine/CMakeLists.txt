# ------- ENGINE -------

project(Engine LANGUAGES CXX)

add_subdirectory(vendor/raylib)

file(GLOB IMGUI_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rlImGui/imgui_impl_raylib.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rlImGui/imgui_impl_raylib.h
)

add_library(imgui STATIC
        ${IMGUI_SOURCE}
)

target_include_directories(imgui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rlImGui
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src
)

file(GLOB_RECURSE ENGINE_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

list(APPEND ENGINE_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Core/ImGuiLayer.cpp)
list(REMOVE_DUPLICATES ENGINE_SOURCE)

add_library(${PROJECT_NAME} STATIC
	${ENGINE_SOURCE}	
)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/rlImGui>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	imgui
    raylib
    opengl32
    gdi32
    winmm
    psapi
)