cmake_minimum_required(VERSION 3.10)
project(KataGoGame CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR})
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(katago_game main.cpp)

target_link_libraries(katago_game ws2_32)

if(WIN32)
    target_compile_options(katago_game PRIVATE /W4)
else()
    target_compile_options(katago_game PRIVATE -Wall -Wextra)
endif()

install(TARGETS katago_game DESTINATION bin)
