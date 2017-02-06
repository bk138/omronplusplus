cmake_minimum_required(VERSION 3.0)
project(omron++ LANGUAGES C)

include(CheckFunctionExists)

set(target_name omron++)
set(version 1.0rc)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

configure_file(config.h_cmake_in config.h)

find_package(SDL)
find_package(SDL_gfx)
if(UNIX)
  set(MATH_LIBRARY m) # for sin()
endif(UNIX)


include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SDL_INCLUDE_DIR}
    ${SDLGFX_INCLUDE_DIR}
)

set(sources
    src/cfg.c
    src/cfg.h
    src/input.c
    src/input.h
    src/ki.c
    src/ki.h
    src/lexwrite.c
    src/lexwrite.h
    src/menu.c
    src/menu.h
    src/sound.c
    src/sound.h
    src/util.c
    src/util.h
    src/video.c
    src/video.h
    src/main.c
)

add_executable(${target_name} ${sources})

target_link_libraries(
    ${target_name}
    ${SDL_LIBRARY}
    ${SDLGFX_LIBRARY}
    ${MATH_LIBRARY}
)
