# Project description

cmake_minimum_required(VERSION "3.14")

set(PROJECT_NAME "Match3SDL2")
set(MAIN_EXECUTABLE_NAME "Match3")
set(UNIT_TESTS_NAME "Match3UnitTests")
set(EXECUTABLES "${MAIN_EXECUTABLE_NAME}" "${UNIT_TESTS_NAME}")

project(${PROJECT_NAME}
    VERSION "1.0.0"
    LANGUAGES "CXX"
)


# Configure dependency subprojects

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(EXT_CODE "external-code-dependencies")
set(EXT_DYNAMIC "external-dynamic-dependencies")
set(EXT_DYNAMIC_DEV "${EXT_DYNAMIC}/development")
set(EXT_DYNAMIC_REL "${EXT_DYNAMIC}/release")

## SDL2
IF(WIN32)
    set(SDL2 "${EXT_DYNAMIC_DEV}/SDL2" "${EXT_DYNAMIC_DEV}/SDL2_image")
ENDIF()

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

## Catch2
find_package(Catch2 REQUIRED)

## nlohmann_json
set(NLOHMANN_JSON_SUBPROJECT "${EXT_CODE}/nlohmann_json")
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory("${NLOHMANN_JSON_SUBPROJECT}")

IF(WIN32)
    set(SDL2_RELEASE_FILES "SDL2/SDL2.dll" "SDL2/README-SDL.txt")
    set(SDL2_image_RELEASE_FILES
        "SDL2_image/libjpeg-9.dll" "SDL2_image/libpng16-16.dll" "SDL2_image/zlib1.dll"
        "SDL2_image/LICENSE.jpeg.txt" "SDL2_image/LICENSE.png.txt" "SDL2_image/LICENSE.zlib.txt"
        "SDL2_image/SDL2_image.dll" "SDL2_image/README.txt"
    )
ENDIF()

## Boost
IF(APPLE)
    set(boost_min_ver 1.50.0)
    set(boost_libs system filesystem)
    find_package(Boost ${boost_min_ver})

    IF(Boost_FOUND)
        find_package(Boost ${boost_min_ver} COMPONENTS ${boost_libs})
    ENDIF()
ENDIF()

# Describe build targets

include(CTest)
include(Catch)

set(EXECUTABLE_INCLUDE_DIRECTORIES
    PRIVATE "${CMAKE_SOURCE_DIR}/src"
    PRIVATE "${SDL2_INCLUDE_DIR}"
    PRIVATE "${SDL2_IMAGE_INCLUDE_DIR}"
)
set(EXECUTABLE_LINK_LIBRARIES
    "nlohmann_json::nlohmann_json" # provided by NLOHMANN_JSON_SUBPROJECT
    "${SDL2_LIBRARY}" # provided by SDL2
    "${SDL2_IMAGE_LIBRARY}" # provided by SDL2_image
)
IF(APPLE)
    set(EXECUTABLE_LINK_LIBRARIES "${Boost_LIBRARIES}" ${EXECUTABLE_LINK_LIBRARIES})
ENDIF()

## Main executable

add_executable("${MAIN_EXECUTABLE_NAME}")
set_property(TARGET "${MAIN_EXECUTABLE_NAME}" PROPERTY CXX_STANDARD 17)

target_include_directories("${MAIN_EXECUTABLE_NAME}"
    ${EXECUTABLE_INCLUDE_DIRECTORIES}
)

target_link_libraries("${MAIN_EXECUTABLE_NAME}"
    ${EXECUTABLE_LINK_LIBRARIES}
)

## Unit tests executable

IF(NOT APPLE)
    add_executable("${UNIT_TESTS_NAME}")
    set_property(TARGET "${UNIT_TESTS_NAME}" PROPERTY CXX_STANDARD 17)

    target_include_directories("${UNIT_TESTS_NAME}"
        ${EXECUTABLE_INCLUDE_DIRECTORIES}
        "${CMAKE_SOURCE_DIR}/test"
    )

    target_link_libraries("${UNIT_TESTS_NAME}"
        ${EXECUTABLE_LINK_LIBRARIES}
        "Catch2::Catch2" # provided by Catch2
    )
ENDIF()

## In windows, copy the release libraries into the target's directory

IF(WIN32)
    set(DYNAMIC_LIBRARY_EXTENSIONS "dll" "txt")

    FOREACH(EXECUTABLE_NAME IN LISTS EXECUTABLES)
        FOREACH(RELEASE_FILE IN LISTS SDL2_RELEASE_FILES SDL2_image_RELEASE_FILES)
            add_custom_command(
                TARGET "${EXECUTABLE_NAME}"
                POST_BUILD
                COMMAND "${CMAKE_COMMAND}" "-E" "copy"
                        "${CMAKE_SOURCE_DIR}/${EXT_DYNAMIC_REL}/${RELEASE_FILE}"
                        "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>"
            )
        ENDFOREACH()
    ENDFOREACH()
ENDIF()


# Include all C++ source files

file(GLOB_RECURSE MAIN_EXECUTABLE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
target_sources("${MAIN_EXECUTABLE_NAME}" PRIVATE "${MAIN_EXECUTABLE_SOURCES}")

IF(NOT APPLE)
    file(GLOB_RECURSE UNIT_TESTS_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "test/*.cpp")
    target_sources("${UNIT_TESTS_NAME}" PRIVATE "${UNIT_TESTS_SOURCES}")
ENDIF()


# Copy resource files (images, json, etc...)

FOREACH(EXECUTABLE_NAME IN LISTS EXECUTABLES)
    add_custom_command(
        TARGET "${MAIN_EXECUTABLE_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" "-E" "copy_directory"
                "${CMAKE_SOURCE_DIR}/runtime-resources"
                "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>"
    )
ENDFOREACH()

IF(NOT APPLE)
    catch_discover_tests("${UNIT_TESTS_NAME}")
ENDIF()
