# ------ WORKSPACE -------

cmake_minimum_required(VERSION 3.31)
project(RayTracer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	add_compile_options(/MP)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH "x64")
else()
	set(ARCH "x86")
endif()

set(OUTPUT_DIR ${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}-${ARCH})

# Set custom output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${OUTPUT_DIR}/${PROJECT_NAME})

add_subdirectory(Engine)
add_subdirectory(Sandbox)