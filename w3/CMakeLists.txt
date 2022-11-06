cmake_minimum_required(VERSION 3.20)

project(w3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE HW3_SOURCES1 . ./*.[ch]pp)
file(GLOB_RECURSE HW3_SOURCES2 . ./*.[ch])

add_executable(hw3 ${HW3_SOURCES1} ${HW3_SOURCES2})
target_link_libraries(hw3 PUBLIC project_options project_warnings)
target_link_libraries(hw3 PUBLIC raylib flecs::flecs_static)


add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${w3_SOURCE_DIR}/assets ${w3_BINARY_DIR}/assets
)
add_dependencies(hw3 copy_assets)
