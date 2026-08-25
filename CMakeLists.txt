cmake_minimum_required(VERSION 3.20)
project(Tangents VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Wpedantic")

# Find dependencies
find_package(SFML 3 COMPONENTS graphics window audio system REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(/usr/local/include)

# Link directories
link_directories(/usr/local/lib)

# Main executable
add_executable(tangents
    src/main.cpp
    src/script_parser.cpp
    src/story_engine.cpp
    src/ui_renderer.cpp
    src/audio_manager.cpp
)

target_link_libraries(tangents
    sfml-graphics
    sfml-window
    sfml-audio
    sfml-system
)

# Testing
enable_testing()

# Catch2 tests (if available)
find_package(Catch2 3 QUIET)
if(Catch2_FOUND)
    add_executable(tests
        tests/test_script_parser.cpp
        src/script_parser.cpp
        src/story_engine.cpp
    )
    target_link_libraries(tests Catch2::Catch2WithMain)
    add_test(NAME TangentsTests COMMAND tests)
else()
    message(STATUS "Catch2 not found - tests will not be built. Install with: brew install catch2")
endif()

# Linting
find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    add_custom_target(lint
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} src/*.cpp include/*.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# Set output directory
set_target_properties(tangents PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
