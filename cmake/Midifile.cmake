SET(midifile_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/midifile/include)

FILE(GLOB midifile_SOURCES
    ${PROJECT_SOURCE_DIR}/midifile/src/*.cpp)

ADD_LIBRARY(midifile STATIC
    ${midifile_SOURCES})

TARGET_INCLUDE_DIRECTORIES(midifile PRIVATE
    ${midifile_INCLUDE_DIRS})
