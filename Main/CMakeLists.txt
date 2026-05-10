message(STATUS "[Module] Start to process ${MAIN_NAME} module.")

################################################################################
# Read source files
################################################################################
file(GLOB ENGINE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
file(GLOB ENGINE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(ENGINE_FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

################################################################################
# Repackiging source files
################################################################################
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_FILES})

################################################################################
# Add library target
################################################################################
add_library(${MAIN_NAME} STATIC ${ENGINE_FILES})

################################################################################
# Set target properties
################################################################################
target_link_libraries(
    ${MAIN_NAME}
    PRIVATE
    COMPILER_WARNING_FLAGS
    PUBLIC
    ${THIRD_PARTY_NAME}
    ${CORE_NAME}
    ${PLATFORMS_NAME}
    )
target_include_directories(
    ${MAIN_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

    
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
# Read source files
################################################################################
file(GLOB PLATFORM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/*.cc)
file(GLOB PLATFORM_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/*.h)

set(PLATFORM_FILES ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})

################################################################################
# Repackiging source files
################################################################################
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER} FILES ${PLATFORM_FILES})

################################################################################
# Add executable target
################################################################################
# Windows
if (WIN32)
  add_executable(${MAIN_PROJECT_NAME} WIN32 ${PLATFORM_FILES})
  set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${MAIN_PROJECT_NAME})
  # Apple
elseif(APPLE)
  message(FATAL_ERROR "This project only supports windows environment currently.")
  add_executable(${MAIN_PROJECT_NAME} ${PLATFORM_FILES})
# Linux
elseif(UNIX AND NOT APPLE)
  message(FATAL_ERROR "This project only supports windows environment currently.")
  add_executable(${MAIN_PROJECT_NAME} ${PLATFORM_FILES})
endif()

################################################################################
# Set target properties
################################################################################
target_link_libraries(
  ${MAIN_PROJECT_NAME}
  PRIVATE
  COMPILER_WARNING_FLAGS
  PUBLIC  
  ${THIRD_PARTY_NAME}
  ${CORE_NAME}
  ${PLATFORMS_NAME} 
  ${MAIN_NAME}
)

target_include_directories(
  ${MAIN_PROJECT_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

message(STATUS "[Module] Processing ${MAIN_NAME} module finished.\n")
