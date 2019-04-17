## Project description

cmake_minimum_required(VERSION "3.14")

set(PROJECT_NAME "Match3SDL2")
set(MAIN_EXECUTABLE_NAME "Match3")
set(UNIT_TESTS_NAME "Match3UnitTests")
set(EXECUTABLES "${MAIN_EXECUTABLE_NAME}" "${UNIT_TESTS_NAME}")

project(${PROJECT_NAME}
    VERSION "0.0.0"
    LANGUAGES "CXX"
)


## Configure dependency subprojects

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(Catch2 REQUIRED)

set(EXTERNAL_CODE "external-code-dependencies")
set(EXTERNAL_DYNAMIC "external-dynamic-dependencies")

if(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIB_PATH_SUFFIX "lib/x64")
    else()
        set(LIB_PATH_SUFFIX "lib/x86")
    endif()
else()
    set(LIB_PATH_SUFFIX "lib")
endif()

# nlohmann_json
set(NLOHMANN_JSON_SUBPROJECT "${EXTERNAL_CODE}/nlohmann_json")
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory("${NLOHMANN_JSON_SUBPROJECT}")

# SDL2 and its modules
# TODO: this is pretty ugly, eventually get a FindSDL2.cmake etc...
find_library(SDL2_LIBRARY
    NAMES "SDL2"
    HINTS "${EXTERNAL_DYNAMIC}/SDL2"
    PATH_SUFFIXES "${LIB_PATH_SUFFIX}"
)
find_library(SDL2_MAIN_LIBRARY
    NAMES "SDL2main"
    HINTS "${EXTERNAL_DYNAMIC}/SDL2"
    PATH_SUFFIXES "${LIB_PATH_SUFFIX}"
)
set(SDL2_BINARIES "SDL2")

find_library(SDL2_IMAGE_LIBRARY
    NAMES "SDL2_image"
    HINTS "${EXTERNAL_DYNAMIC}/SDL2_image"
    PATH_SUFFIXES "${LIB_PATH_SUFFIX}"
)
set(SDL2_image_BINARIES "libjpeg-9" "libpng16-16" "libtiff-5" "libwebp-7" "SDL2_image" "zlib1")

find_library(SDL2_TTF_LIBRARY
    NAMES "SDL2_ttf"
    HINTS "${EXTERNAL_DYNAMIC}/SDL2_ttf"
    PATH_SUFFIXES "${LIB_PATH_SUFFIX}"
)
set(SDL2_ttf_BINARIES "libfreetype-6" "SDL2_ttf")

find_library(SDL2_MIXER_LIBRARY
    NAMES "SDL2_mixer"
    HINTS "${EXTERNAL_DYNAMIC}/SDL2_mixer"
    PATH_SUFFIXES "${LIB_PATH_SUFFIX}"
)
set(SDL2_mixer_BINARIES "libFLAC-8" "libmodplug-1" "libmpg123-0" "libogg-0" "libopus-0" "libopusfile-0" "libvorbis-0" "libvorbisfile-3" "SDL2_mixer")

set(SDL2_LIBRARIES "SDL2" "SDL2_image" "SDL2_ttf" "SDL2_mixer")
# end TODO


## Describe build targets

include(CTest)
include(Catch)

set(EXECUTABLE_INCLUDE_DIRECTORIES
    PRIVATE "${EXTERNAL_DYNAMIC}/SDL2/include"
    PRIVATE "${EXTERNAL_DYNAMIC}/SDL2_image/include"
    PRIVATE "${EXTERNAL_DYNAMIC}/SDL2_ttf/include"
    PRIVATE "${EXTERNAL_DYNAMIC}/SDL2_mixer/include"
)
set(EXECUTABLE_LINK_LIBRARIES
    "nlohmann_json::nlohmann_json" # provided by NLOHMANN_JSON_SUBPROJECT
    "${SDL2_LIBRARY}"
    "${SDL2_MAIN_LIBRARY}"
    "${SDL2_IMAGE_LIBRARY}"
    "${SDL2_TTF_LIBRARY}"
    "${SDL2_MIXER_LIBRARY}"
)

# Main executable
add_executable("${MAIN_EXECUTABLE_NAME}")
set_property(TARGET "${MAIN_EXECUTABLE_NAME}" PROPERTY CXX_STANDARD 17)

target_include_directories("${MAIN_EXECUTABLE_NAME}"
    ${EXECUTABLE_INCLUDE_DIRECTORIES}
)

target_link_libraries("${MAIN_EXECUTABLE_NAME}"
    ${EXECUTABLE_LINK_LIBRARIES}
)

# Unit tests executable
add_executable("${UNIT_TESTS_NAME}")
set_property(TARGET "${UNIT_TESTS_NAME}" PROPERTY CXX_STANDARD 17)

target_include_directories("${UNIT_TESTS_NAME}"
    ${EXECUTABLE_INCLUDE_DIRECTORIES}
)

target_link_libraries("${UNIT_TESTS_NAME}"
    ${EXECUTABLE_LINK_LIBRARIES}
    "Catch2::Catch2" # provided by Catch2
)

# TODO: remove this ugly code too
#   should be simple with the Find*.cmake etc... modules
set(DYNAMIC_LIBRARY_EXTENSIONS "dll" "dylib" "so")

FOREACH(EXECUTABLE_NAME IN LISTS EXECUTABLES)
    FOREACH(EXTENSION IN LISTS DYNAMIC_LIBRARY_EXTENSIONS)
        FOREACH(SDL2_LIBRARY IN LISTS SDL2_LIBRARIES)
            FOREACH(BINARY IN LISTS "${SDL2_LIBRARY}_BINARIES")
                set(LIB_BINARY_FULL_PATH "${CMAKE_SOURCE_DIR}/${EXTERNAL_DYNAMIC}/${SDL2_LIBRARY}/${LIB_PATH_SUFFIX}/${BINARY}.${EXTENSION}")

                IF(EXISTS "${LIB_BINARY_FULL_PATH}")
                    add_custom_command(
                        TARGET "${EXECUTABLE_NAME}"
                        POST_BUILD
                        COMMAND "${CMAKE_COMMAND}" "-E" "copy"
                                "${LIB_BINARY_FULL_PATH}"
                                "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>"
                    )
                ENDIF()
            ENDFOREACH()
        ENDFOREACH()
    ENDFOREACH()
ENDFOREACH()
# end TODO

file(GLOB_RECURSE MAIN_EXECUTABLE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
target_sources("${MAIN_EXECUTABLE_NAME}" PRIVATE "${MAIN_EXECUTABLE_SOURCES}")

file(GLOB_RECURSE UNIT_TESTS_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "test/*.cpp")
target_sources("${UNIT_TESTS_NAME}" PRIVATE "${UNIT_TESTS_SOURCES}")

FOREACH(EXECUTABLE_NAME IN LISTS EXECUTABLES)
    add_custom_command(
        TARGET "${MAIN_EXECUTABLE_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory"
                "${CMAKE_SOURCE_DIR}/runtime-resources"
                "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>"
    )
ENDFOREACH()

catch_discover_tests("${UNIT_TESTS_NAME}")
