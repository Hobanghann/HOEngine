##########################################################
# CMake settings
##########################################################

cmake_minimum_required(VERSION 3.30)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

include(FetchContent)

set(MAIN_PROJECT_NAME HOEngine)

project(${MAIN_PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(DISABLE_DOWNLOAD_EXTERNAL_LIBRARIES "Disable download external libraries" OFF)

option(ENABLE_TEST "Enable test" ON)

##########################################################
# Set Compiler Options
##########################################################
add_library(COMPILER_FLAGS INTERFACE)

if (MSVC)
    target_compile_options(
        COMPILER_FLAGS
        INTERFACE
        /WX             # -Werror
        /W4             # -Wall -Wextra
        /w14242         # -Wconversion
        /w14254         # -Wconversion
        /w14287         # -Wsign-conversion
        /w14296         # -Wtautological-compare
        /w14456         # -Wshadow
        /w14457         # -Wshadow
        /w14458         # -Wshadow
        /w14459         # -Wshadow
        /w14668         # -Wundef
        /w14702         # -Wunreachable-code
        /w14263         # -Woverloaded-virtual
        /w14264         # -Woverloaded-virtual
        /w14266         # -Woverloaded-virtual
        /wd4100         # -Wno-unused-parameter
        /wd4201         # allow anonymous struct, union
        /wd4723         # disable: potential divide by zero (false positive)
        /wd4324         # disable: redundant padding warning
        /permissive-
        /Zc:__cplusplus
        /EHsc
        /D_HAS_EXCEPTIONS=0
    )

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(
            COMPILER_FLAGS
            INTERFACE
            -Werror
            -Wall
            -Wextra
            -Wconversion
            -Wsign-conversion
            -Wold-style-cast
            -Wshadow
            -Wundef
            -Wunreachable-code
            -Wstrict-aliasing
            -Wnull-dereference
            -Wdouble-promotion
            -Wcast-qual
            -Wcast-align
            -fms-extensions
            -Woverloaded-virtual
            -Wmissing-prototypes
            -Wtautological-compare
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-unused-parameter
            -Wno-gnu-anonymous-struct   # allow anonymous struct/union
            -Wno-double-promotion       # allow float to double promotion
            -Wno-sign-conversion
            -fno-exceptions
        )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(FATAL_ERROR
        "GCC is not supported by HOEngine due to its restrictions on "
        "anonymous aggregates with non-POD members. "
        "Please use Clang or MSVC instead."
    )
else()
    message(WARNING
        "Unknown compiler '${CMAKE_CXX_COMPILER_ID}'. "
        "HOEngine officially supports only MSVC and Clang. "
        "Compilation may work, but is not tested."
    )
endif ()

##########################################################
# Set module's Name
##########################################################
set(THIRD_PARTY_NAME Lib)
set(PLATFORMS_NAME Platforms)
set(CORE_NAME Core)
set(SUBSYSTEMS_NAME Subsystems)
set(MAIN_NAME Main)

##########################################################
# Process modules
##########################################################
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${THIRD_PARTY_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORMS_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${CORE_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${SUBSYSTEMS_NAME})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_NAME})

##########################################################
# Process tests
##########################################################
if(ENABLE_TEST)
enable_testing()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Test)
endif()