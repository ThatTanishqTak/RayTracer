# ------- SANDBOX -------

project(Sandbox LANGUAGES CXX)

file(GLOB_RECURSE SANDBOX_SOURCE
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.h	
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp	
)

add_executable(${PROJECT_NAME} ${SANDBOX_SOURCE})

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
    $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PRIVATE ${CMAKE_SOURCE_DIR}/Engine/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Engine
)