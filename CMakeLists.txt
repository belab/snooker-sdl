cmake_minimum_required(VERSION 3.6)
project(snooker-sdl VERSION 0.1.0 LANGUAGES C CXX)

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/std:c++17 /W3 /permissive- /TP>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2 SDL2_ttf::SDL2_ttf)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_SOURCE_DIR}/lcd.14.otf" $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
)
