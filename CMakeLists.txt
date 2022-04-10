CMAKE_MINIMUM_REQUIRED(VERSION 3.17)

PROJECT(XMIDIX VERSION 1.0 LANGUAGES CXX)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)

SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

FIND_PACKAGE(Qt5 COMPONENTS Widgets REQUIRED)
FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(spdlog REQUIRED)
FIND_PACKAGE(fmt REQUIRED)

INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(ALSA REQUIRED alsa)

INCLUDE(cmake/Midifile.cmake)

SET(PROJECT_SOURCES
    xmidix.cc
    xmidix_player.cc
    xmidix_player.h
    xmidix_file.cc
    xmidix_file.h
    xmidix_seq.cc
    xmidix_seq.h
    xmidix_config.cc
    xmidix_config.h
    xmidix_playlist.cc
    xmidix_playlist.h
    resources/resources.qrc
    xmidix_vu.cc
    xmidix_vu.h
    xmidix_piano.cc
    xmidix_piano.h
    )

ADD_EXECUTABLE(xmidix ${PROJECT_SOURCES})

TARGET_INCLUDE_DIRECTORIES(xmidix PRIVATE
    ${midifile_INCLUDE_DIRS}
    ${ALSA_INCLUDE_DIRS}
    )

TARGET_LINK_LIBRARIES(xmidix PRIVATE
    Qt5::Widgets
    Threads::Threads
    spdlog::spdlog
    fmt::fmt
    midifile
    ${ALSA_LIBRARIES}
    )

INSTALL(TARGETS xmidix DESTINATION bin)
INSTALL(FILES resources/xmidix.desktop DESTINATION share/applications)

SET(CPACK_PACKAGE_CONTACT EMAILBEN145@gmail.com)
SET(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "hardware MIDI player")
SET(CPACK_DEBIAN_PACKAGE_SECTION sound)
SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
INCLUDE(CPack)
