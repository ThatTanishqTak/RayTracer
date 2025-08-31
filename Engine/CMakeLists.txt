# ------- ENGINE -------

project(Engine LANGUAGES CXX)

add_subdirectory(vendor/raylib)

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
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    raylib
    opengl32
    gdi32
    winmm
)