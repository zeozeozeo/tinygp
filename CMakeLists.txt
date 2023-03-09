cmake_minimum_required(VERSION 3.5)

project(tinygp LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

set(SOURCES
    examples/main.c
)

add_executable(tinygp ${SOURCES})

include_directories(${CMAKE_SOURCE_DIR} ${SDL2_INCLUDE_DIRS})
target_link_libraries(tinygp ${SDL2_LIBRARIES} m GLESv2 EGL)

install(TARGETS tinygp
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
