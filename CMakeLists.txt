cmake_minimum_required(VERSION 3.25)
project(Game2048)

set(CMAKE_CXX_STANDARD 17)
set(BIN_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(GLOB_RECURSE SOURCES
        src/*.cpp
        )

file(GLOB_RECURSE INCLUDE
        src/*.h
        src/*.hpp
        )

add_executable(Game2048 ${SOURCES} ${INCLUDE})

# set output directories for all builds (Debug, Release, etc.)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set_target_properties(Game2048
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${BIN_DIRECTORY}
            )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set_target_properties(Game2048
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIRECTORY}
        )

set_target_properties(Game2048
        PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY ${BIN_DIRECTORY}
        )

add_subdirectory(Vendor/raylib)

# Dependencies - raylib
target_link_libraries(Game2048 raylib)
