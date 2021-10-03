cmake_minimum_required(VERSION 3.14)

project(qspcplay VERSION 0.1 LANGUAGES CXX)

## Configure global settings
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    add_compile_definitions(UNICODE _UNICODE)
    # rtaudio_c.cpp uses strncpy and the like. I don't actually use that file lol
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    if (MSVC)
        add_compile_options(/EHsc)
        add_compile_options(/permissive-)
    endif ()
endif ()

if (MSVC)
    list(APPEND options-3rdparty "/W3")
endif ()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    list(APPEND options
        -Wall -Wextra -Wconversion -Wsign-conversion
        -Wmissing-declarations
        -Wno-unused-function -Wno-unused-parameter
        -Werror=return-type
    )
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND options
            -Wtautological-unsigned-zero-compare
            -Wmissing-variable-declarations
        )
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND options
        /Zc:preprocessor
        /W3
        # I want to use /W4, but I get *far* too many false-positive C4244 warnings
        # when passing integer literals into functions.
        # And /w34244 doesn't help; it means "enable C4244 at /W3",
        # not "enable the /W3 version of C4244".
        /wd4100  # allow unused parameters
        /wd4505  # allow unused functions
        /wd4456 /wd4457  # allow shadowing
    )
endif ()

# Strip bloat from release Windows MinGW builds.
if (
    WIN32
    AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
    AND CMAKE_BUILD_TYPE STREQUAL "Release"
)
    add_link_options(-Wl,--strip-all -Wl,--insert-timestamp -Wl,--gc-sections -Wl,--discard-all)
endif()


## Add dependencies

include_directories(src)
#include_directories(SYSTEM 3rdparty)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick QuickControls2 Sql REQUIRED)
set(Qt "Qt${QT_VERSION_MAJOR}")
find_package(${Qt} COMPONENTS Core Quick QuickControls2 Sql REQUIRED)

add_subdirectory("3rdparty/fmt" EXCLUDE_FROM_ALL)
add_subdirectory("3rdparty/STX" EXCLUDE_FROM_ALL)


## Application

set(PROJECT_SOURCES
    src/main.cpp
    src/qml.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qspcplay
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qspcplay APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(qspcplay SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qspcplay
          ${PROJECT_SOURCES}
        )
    endif()
endif()

target_compile_definitions(qspcplay
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
target_compile_options(qspcplay PRIVATE "${options}")
target_link_libraries(qspcplay
  PRIVATE ${Qt}::Core ${Qt}::Quick ${Qt}::QuickControls2 ${Qt}::Sql
  PRIVATE fmt::fmt stx
)

set_target_properties(qspcplay PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(qspcplay)
    qt_finalize_executable(qspcplay)
endif()
