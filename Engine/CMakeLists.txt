# ------- ENGINE -------

project(Engine LANGUAGES CXX)

add_subdirectory(vendor/raylib)

file(GLOB IMGUI_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
)

add_library(imgui STATIC
        ${IMGUI_SOURCE}
)

target_include_directories(imgui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
)

file(GLOB_RECURSE ENGINE_SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h	
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp	
)

add_library(${PROJECT_NAME} STATIC
	${ENGINE_SOURCE}	
)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>	
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/raylib/src>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	imgui
    raylib
    opengl32
    gdi32
    winmm
    psapi
)