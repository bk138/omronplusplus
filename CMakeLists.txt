cmake_minimum_required(VERSION 3.0)
project(omron++
    VERSION 1.0.0
    LANGUAGES C)

include(CheckFunctionExists)

set(target_name omron++)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(SDL REQUIRED)
find_package(SDL_gfx REQUIRED)
find_package(OpenGL)
if(UNIX)
  set(MATH_LIBRARY m) # for sin()
endif(UNIX)

configure_file(config.h_cmake_in config.h)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SDL_INCLUDE_DIR}
    ${SDLGFX_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
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
    ${SDLMAIN_LIBRARY}
    ${SDLGFX_LIBRARY}
    ${MATH_LIBRARY}
    ${OPENGL_LIBRARIES}
)


#
# install directives
#

include(GNUInstallDirs)

install(TARGETS ${target_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES src/omronplusplus.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES README.md AUTHORS ChangeLog NEWS DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES src/omron++.png src/omron++_32.xpm DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)

#
# packaging directives
#

set(CPACK_STRIP_FILES true)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})


if(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Christian Beier <dontmind@freeshell.org>")
    set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "The simplistic peace-endowing warfare simulation.
 In omron++, you have up to four armies of pixels fighting each other.
 You can choose the number of soldiers on each side and some other parameters.
 If you give them a certain 'mobsize', they will form groups of pixels (called
 mobs) and go hunting together.")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
endif(UNIX)

include(CPack)

