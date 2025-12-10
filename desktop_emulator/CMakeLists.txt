cmake_minimum_required(VERSION 3.16)
project(PocketMage_PDA_Emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable debug symbols by default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Add debug flags
if(CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Building in Debug mode with debug symbols")
  if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od /DEBUG:FULL")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG:FULL")
  else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
  endif()
endif()

# ---------------------------
# SDL2 / SDL2_ttf discovery
# ---------------------------
set(_sdl_config_ok FALSE)

# Prefer CMake CONFIG packages on Windows (vcpkg or official SDL2 devel zips)
if (WIN32)
  find_package(SDL2 CONFIG QUIET)
  if (TARGET SDL2::SDL2)
    set(_sdl_config_ok TRUE)
    find_package(SDL2_ttf CONFIG REQUIRED)
  endif()
endif()

# Fallback to pkg-config (macOS/Linux)
if (NOT _sdl_config_ok)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL2 REQUIRED sdl2)
  pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

  add_library(SDL2::SDL2 INTERFACE IMPORTED)
  target_include_directories(SDL2::SDL2 INTERFACE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(SDL2::SDL2 INTERFACE ${SDL2_LIBRARIES})

  add_library(SDL2_ttf::SDL2_ttf INTERFACE IMPORTED)
  target_include_directories(SDL2_ttf::SDL2_ttf INTERFACE ${SDL2_TTF_INCLUDE_DIRS})
  target_link_libraries(SDL2_ttf::SDL2_ttf INTERFACE ${SDL2_TTF_LIBRARIES})
endif()

# Homebrew helper for macOS 
if(APPLE)
  execute_process(COMMAND brew --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
  list(APPEND CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}")
  include_directories("${HOMEBREW_PREFIX}/include")
  link_directories("${HOMEBREW_PREFIX}/lib")
endif()

# ---------------------------
# PocketMage source paths
# ---------------------------
set(POCKETMAGE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../Code/PocketMage_V3)
set(POCKETMAGE_SRC ${POCKETMAGE_ROOT}/src)
set(POCKETMAGE_APPS ${POCKETMAGE_SRC}/OS_APPS)
set(POCKETMAGE_INCLUDE ${POCKETMAGE_ROOT}/include)
set(POCKETMAGE_LIB ${POCKETMAGE_ROOT}/lib/PocketMage)
set(POCKETMAGE_LIB_INCLUDE ${POCKETMAGE_LIB}/include)
set(POCKETMAGE_LIB_SRC ${POCKETMAGE_LIB}/src)

# ---------------------------
# PocketMage library source files
# ---------------------------
# PocketMage library sources are excluded - they use ESP32-specific APIs
# The emulator provides its own implementations via pocketmage_shim.cpp
set(POCKETMAGE_LIB_SOURCES
    ${POCKETMAGE_LIB_SRC}/libAssets.cpp
)

# ---------------------------
# PocketMage source files
# ---------------------------
set(POCKETMAGE_SOURCES
    ${POCKETMAGE_SRC}/PocketMageV3.cpp
    ${POCKETMAGE_SRC}/globals.cpp
    ${POCKETMAGE_SRC}/assets.cpp
    ${POCKETMAGE_APPS}/HOME.cpp
    ${POCKETMAGE_APPS}/TXT.cpp
    ${POCKETMAGE_APPS}/TXT_NEW.cpp
    ${POCKETMAGE_APPS}/FILEWIZ.cpp
    ${POCKETMAGE_APPS}/TASKS.cpp
    ${POCKETMAGE_APPS}/CALENDAR.cpp
    ${POCKETMAGE_APPS}/SETTINGS.cpp
    ${POCKETMAGE_APPS}/JOURNAL.cpp
    ${POCKETMAGE_APPS}/LEXICON.cpp
    ${POCKETMAGE_SRC}/HELLO_WORLD.cpp
    ${POCKETMAGE_SRC}/AstraLuaApp_APP.cpp
    ${POCKETMAGE_SRC}/FlashCardApp_APP.cpp
    ${POCKETMAGE_SRC}/GlucoseApp_APP.cpp
    ${POCKETMAGE_SRC}/MusicApp_APP.cpp
    ${POCKETMAGE_SRC}/StarterApp_APP.cpp      # Example app - see docs/APP_DEVELOPMENT.md
    ${POCKETMAGE_APPS}/APPLAUNCHER.cpp        # Desktop emulator app launcher
    # ========================================
    # ADD YOUR CUSTOM APPS HERE
    # Use ${POCKETMAGE_SRC}/MYAPP.cpp for files in Code/PocketMage_V3/src/
    # Use ${POCKETMAGE_APPS}/MYAPP.cpp for files in Code/PocketMage_V3/src/OS_APPS/
    # ========================================
    # APPLOADER.cpp excluded - requires ESP32 OTA functionality
    # USB.cpp and BT.cpp excluded - ESP32-specific or empty
)

# ---------------------------
# Emulator sources (common)
# ---------------------------
set(EMULATOR_SOURCES_COMMON
    src/main.cpp
    src/desktop_display_sdl2.cpp
    src/hardware_shim.cpp
    src/pocketmage_shim.cpp
    src/pocketmage_stubs.cpp
    src/oled_service.cpp
    src/gfx_fonts.cpp
    src/minilua.c
)

 # Platform-specific display backend (SDL2-based)
 if(WIN32)
     message(STATUS "Building for Windows - using SDL2 Windows display backend")
     set(EMULATOR_SOURCES_PLATFORM src/desktop_display_sdl2_windows.cpp)
 elseif(APPLE)
     message(STATUS "Building for macOS - using SDL2 macOS display backend")
     set(EMULATOR_SOURCES_PLATFORM src/desktop_display_macos.cpp)
 elseif(UNIX)
     message(STATUS "Building for Linux - using SDL2 Linux display backend")
     set(EMULATOR_SOURCES_PLATFORM src/desktop_display_linux.cpp)
 else()
     message(FATAL_ERROR "Unsupported platform")
 endif()

set(EMULATOR_SOURCES ${EMULATOR_SOURCES_COMMON} ${EMULATOR_SOURCES_PLATFORM})

# ---------------------------
# Build executable
# ---------------------------
add_executable(${PROJECT_NAME}
    ${EMULATOR_SOURCES}
    ${POCKETMAGE_LIB_SOURCES}
    ${POCKETMAGE_SOURCES}
)

# Include directories - emulator includes MUST come first to override
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/display
    ${CMAKE_CURRENT_SOURCE_DIR}/include/input
    ${CMAKE_CURRENT_SOURCE_DIR}/include/storage
    ${CMAKE_CURRENT_SOURCE_DIR}/include/esp32
    ${CMAKE_CURRENT_SOURCE_DIR}/include/hardware
    ${CMAKE_CURRENT_SOURCE_DIR}/include/pocketmage
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Fonts  # Emulator font stubs
    ${CMAKE_CURRENT_SOURCE_DIR}/fonts/gfx  # Downloaded Adafruit GFX fonts
    ${POCKETMAGE_INCLUDE}
    ${POCKETMAGE_LIB_INCLUDE}
    ${LIBSSH_INCLUDE_DIRS}  # libssh headers
)

# ---------------------------
# Compile definitions
# ---------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    DESKTOP_EMULATOR
    SDL_MAIN_HANDLED
)

# Platform-specific settings
if(APPLE)
  message(STATUS "Building for macOS")
  target_compile_definitions(${PROJECT_NAME} PRIVATE __APPLE__)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers)
elseif(UNIX)
  message(STATUS "Building for Linux")
  target_compile_definitions(${PROJECT_NAME} PRIVATE __LINUX__)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter)
elseif(WIN32)
  message(STATUS "Building for Windows")
  if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3)
  endif()
endif()

# ---------------------------
# Link libraries
# ---------------------------
set(_sdl_targets SDL2::SDL2 SDL2_ttf::SDL2_ttf)
if (TARGET SDL2::SDL2main)
  list(INSERT _sdl_targets 0 SDL2::SDL2main)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${_sdl_targets})


# ---------------------------
# Post-build: copy assets
# ---------------------------
# Copy data directory
if(EXISTS ${CMAKE_SOURCE_DIR}/data)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
    COMMENT "Copying data directory"
    VERBATIM
  )
endif()

# Copy fonts directory
if(EXISTS ${CMAKE_SOURCE_DIR}/fonts)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/fonts $<TARGET_FILE_DIR:${PROJECT_NAME}>/fonts
    COMMENT "Copying fonts directory"
    VERBATIM
  )
endif()

# ---------------------------
# Tests (optional)
# ---------------------------
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
