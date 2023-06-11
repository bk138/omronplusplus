cmake_minimum_required(VERSION 3.25)
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
    ${SDL_GFX_INCLUDE_DIRS}
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

if(WIN32)
    set(sources ${sources} src/omron++.rc)
endif(WIN32)

if(APPLE)
    set(target_name Omron++)
    set(MACOSX_BUNDLE_COPYRIGHT "© Christian Beier (dontmind@freeshell.org), 2008-2018")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(MACOSX_BUNDLE_ICON_FILE omron++.icns)
    set(sources ${sources} src/omron++.icns)
    set_source_files_properties(src/omron++.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif(APPLE)

add_executable(${target_name} MACOSX_BUNDLE ${sources})

target_link_libraries(
    ${target_name}
    ${SDL_LIBRARY}
    ${SDLMAIN_LIBRARY}
    ${SDL_GFX_LIBRARIES}
    ${MATH_LIBRARY}
    ${OPENGL_LIBRARIES}
)


#
# install directives
#

if(WIN32)
    install(TARGETS ${target_name} RUNTIME DESTINATION bin/)
    install(FILES src/msw/SDL.dll src/msw/SDL_gfx.dll src/msw/README-SDL DESTINATION bin/)
    install(FILES README.md DESTINATION . RENAME Readme.txt)
    install(FILES NEWS DESTINATION . RENAME ReleaseNotes.txt)
    install(FILES COPYING DESTINATION . RENAME License.txt)
endif(WIN32)

if(UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    install(TARGETS ${target_name} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES data/omronplusplus.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES README.md AUTHORS ChangeLog NEWS DESTINATION ${CMAKE_INSTALL_DOCDIR})
    install(FILES data/omron++.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
endif(UNIX AND NOT APPLE)

if(APPLE)
    set(APPS ${CMAKE_CURRENT_BINARY_DIR}/${target_name}.app)  # paths to executables
    set(DIRS "")   # directories to search for prerequisites
    INSTALL(CODE "
set(BU_CHMOD_BUNDLE_ITEMS ON) # as per https://cmake.org/Bug/view.php?id=13833
include(BundleUtilities)
fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
   " COMPONENT Runtime)
    INSTALL(TARGETS ${target_name} BUNDLE DESTINATION . COMPONENT Runtime)
    INSTALL(FILES README.md DESTINATION . RENAME Readme.txt)
endif(APPLE)

#
# packaging directives
#

set(CPACK_STRIP_FILES true)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/COPYING)
set(CPACK_PACKAGE_EXECUTABLES "${target_name};Omron++")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_MUI_ICON ${CMAKE_SOURCE_DIR}/src/omron++.ico)
    set(CPACK_NSIS_MUI_FINISHPAGE_RUN ${target_name})
endif(WIN32)

if(UNIX AND NOT APPLE)
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
endif(UNIX AND NOT APPLE)

if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_PACKAGE_FILE_NAME "${target_name}-${PROJECT_VERSION}")
    set(CPACK_INSTALL_PREFIX .) # for some reason defaults to /usr/local/
endif(APPLE)

include(CPack)

