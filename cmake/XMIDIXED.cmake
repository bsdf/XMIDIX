SET(XMIDIXED_SOURCES
    xmidixed.cc
    xmidix_file.cc
    pw/midifile.c
    )

ADD_EXECUTABLE(xmidixed
    ${XMIDIXED_SOURCES}
    )

TARGET_INCLUDE_DIRECTORIES(xmidixed PRIVATE
    ${ALSA_INCLUDE_DIRS}
    ${SMF_INCLUDE_DIRS}
    )

TARGET_LINK_LIBRARIES(xmidixed PRIVATE
    Qt5::Widgets
    spdlog::spdlog
    fmt::fmt
    ${ALSA_LIBRARIES}
    ${SMF_LIBRARIES}
    )
