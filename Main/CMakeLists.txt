message(STATUS "[Module] Start to process ${MAIN_NAME} module.")

################################################################################
# Read Engine Main source files
################################################################################
file(GLOB ENGINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
file(GLOB ENGINE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(ENGINE_FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

################################################################################
# Repackaging source files
################################################################################
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_FILES})

################################################################################
# Check Platform
################################################################################
# Windows
if (WIN32)
  set(PLATFORM_FOLDER Windows)
# Apple
elseif(APPLE)
  message(FATAL_ERROR "This project only supports windows environment currently.")
  set(PLATFORM_FOLDER Apple)
# Linux
elseif(UNIX AND NOT APPLE)
  message(FATAL_ERROR "This project only supports windows environment currently.")
  set(PLATFORM_FOLDER Linux)
endif()

################################################################################
# Read Platform Main source files
################################################################################
file(GLOB PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/*.cc)
file(GLOB PLATFORM_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/*.h)

set(PLATFORM_FILES ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER} FILES ${PLATFORM_FILES})

################################################################################
# Add multiple executable targets
################################################################################
set(APP_TYPES TestApp ModelViewer Editor Game)

set(MACRO_TestApp      "TEST_APP")
set(MACRO_ModelViewer  "MODEL_VIEWER")
set(MACRO_Editor       "EDITOR")
set(MACRO_Game         "GAME")

foreach(APP_TYPE IN LISTS APP_TYPES)

    set(TARGET_NAME "${APP_TYPE}")

    set(FINAL_MACRO "HO_APP_${MACRO_${APP_TYPE}}")

    # Windows
    if (WIN32)
        add_executable(${TARGET_NAME} WIN32 ${ENGINE_FILES} ${PLATFORM_FILES})
    # Apple
    elseif(APPLE)
        add_executable(${TARGET_NAME} ${ENGINE_FILES} ${PLATFORM_FILES})
    # Linux
    elseif(UNIX AND NOT APPLE)
        add_executable(${TARGET_NAME} ${ENGINE_FILES} ${PLATFORM_FILES})
    endif()

    ################################################################################
    # Set target properties
    ################################################################################
    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE
        COMPILER_FLAGS
        PUBLIC
        ${PLATFORMS_NAME}
        ${ENGINE_NAME}
    )

    target_include_directories(
        ${TARGET_NAME}
        PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE ${FINAL_MACRO})

endforeach()

################################################################################
# Set VS startup project
################################################################################
# set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT "${MAIN_PROJECT_NAME}_Editor")

message(STATUS "[Module] Processing ${MAIN_NAME} module finished.\n")