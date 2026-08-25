cmake_minimum_required(VERSION 3.20)
project(Tangents VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -Wpedantic")

# ============================================================================
# Setup paths for Homebrew and other package managers
# ============================================================================

# Homebrew Intel and Apple Silicon paths
if(APPLE)
    # Apple Silicon (M1/M2/M3)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
        list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/sfml")
    endif()
    # Intel Macs
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        list(APPEND CMAKE_PREFIX_PATH "/usr/local")
        list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/sfml")
    endif()
endif()

# Standard Unix paths
list(APPEND CMAKE_PREFIX_PATH "/usr/local")
list(APPEND CMAKE_PREFIX_PATH "/opt")

# ============================================================================
# Find SFML
# ============================================================================

find_package(SFML 3 COMPONENTS graphics window audio system QUIET)

if(NOT SFML_FOUND)
    message(STATUS "SFML 3.x not found via find_package, trying manual search...")

    # Manual search in common Homebrew locations
    find_library(SFML_GRAPHICS_LIB sfml-graphics PATHS /usr/local/lib /opt/homebrew/lib NO_DEFAULT_PATH)
    find_library(SFML_WINDOW_LIB sfml-window PATHS /usr/local/lib /opt/homebrew/lib NO_DEFAULT_PATH)
    find_library(SFML_AUDIO_LIB sfml-audio PATHS /usr/local/lib /opt/homebrew/lib NO_DEFAULT_PATH)
    find_library(SFML_SYSTEM_LIB sfml-system PATHS /usr/local/lib /opt/homebrew/lib NO_DEFAULT_PATH)

    find_path(SFML_INCLUDE_DIR SFML/Graphics.hpp PATHS /usr/local/include /opt/homebrew/include NO_DEFAULT_PATH)

    if(SFML_GRAPHICS_LIB AND SFML_WINDOW_LIB AND SFML_AUDIO_LIB AND SFML_SYSTEM_LIB AND SFML_INCLUDE_DIR)
        message(STATUS "SFML found manually at ${SFML_INCLUDE_DIR}")
        set(SFML_MANUAL_FOUND TRUE)
    else()
        message(FATAL_ERROR "SFML 3.x not found. Install with: brew install sfml")
    endif()
else()
    message(STATUS "SFML 3.x found via find_package")
endif()

# ============================================================================
# Find toml++ (header-only library)
# ============================================================================

find_path(TOML_INCLUDE_DIR toml.hpp PATHS /usr/local/include /opt/homebrew/include NO_DEFAULT_PATH)

if(NOT TOML_INCLUDE_DIR)
    # Try pkg-config
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(TOML QUIET toml++)
        if(TOML_FOUND)
            set(TOML_INCLUDE_DIR ${TOML_INCLUDE_DIRS})
        endif()
    endif()
endif()

if(NOT TOML_INCLUDE_DIR)
    message(FATAL_ERROR "toml++ not found. Install with: brew install toml-plusplus")
endif()

message(STATUS "toml++ found at ${TOML_INCLUDE_DIR}")

# ============================================================================
# Main executable
# ============================================================================

add_executable(tangents
    src/main.cpp
    src/script_parser.cpp
    src/story_engine.cpp
    src/ui_renderer.cpp
    src/audio_manager.cpp
)

# Include directories
target_include_directories(tangents PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SFML_INCLUDE_DIR}
    ${TOML_INCLUDE_DIR}
    /usr/local/include
    /opt/homebrew/include
)

# Link libraries
if(SFML_FOUND)
    target_link_libraries(tangents PRIVATE SFML::Graphics SFML::Window SFML::Audio SFML::System)
else()
    # Fallback to manual linking
    target_link_libraries(tangents PRIVATE
        ${SFML_GRAPHICS_LIB}
        ${SFML_WINDOW_LIB}
        ${SFML_AUDIO_LIB}
        ${SFML_SYSTEM_LIB}
    )
    target_link_directories(tangents PRIVATE /usr/local/lib /opt/homebrew/lib)
endif()

# ============================================================================
# Testing with Catch2
# ============================================================================

enable_testing()

find_package(Catch2 3 QUIET)
if(Catch2_FOUND)
    add_executable(tests
        tests/test_script_parser.cpp
        src/script_parser.cpp
        src/story_engine.cpp
    )
    target_include_directories(tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${TOML_INCLUDE_DIR}
    )
    target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
    add_test(NAME TangentsTests COMMAND tests)
    message(STATUS "Tests enabled")
else()
    message(STATUS "Catch2 not found - tests will not be built. Install with: brew install catch2")
endif()

# ============================================================================
# Linting target
# ============================================================================

find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    add_custom_target(lint
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} src/*.cpp include/*.h
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    message(STATUS "Lint target available")
else()
    message(STATUS "clang-tidy not found - lint target unavailable")
endif()

# ============================================================================
# Output configuration
# ============================================================================

set_target_properties(tangents PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "Tangents v${PROJECT_VERSION} configured successfully")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
