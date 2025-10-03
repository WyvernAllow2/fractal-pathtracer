cmake_minimum_required(VERSION 3.15)

if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(fractal-pathtracer VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
    src/main.cpp
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
        /permissive-
        /sdl
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

add_subdirectory(deps/glfw)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
)