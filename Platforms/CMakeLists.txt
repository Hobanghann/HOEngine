message(STATUS "[Module] Start to process ${PLATFORMS_NAME} module.")

################################################################################
# define variables
################################################################################
set(HO_PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
set(HO_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets")

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
file(GLOB BASE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
file(GLOB BASE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/*.cc)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/${PLATFORM_FOLDER}/*.h)

set(ALL_FILES ${BASE_SOURCES} ${BASE_HEADERS} ${SOURCES} ${HEADERS})

################################################################################
# Repackiging source files
################################################################################
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_FILES})

################################################################################
# Add library target
################################################################################
add_library(${PLATFORMS_NAME} STATIC ${ALL_FILES})

################################################################################
# Set target properties
################################################################################
target_link_libraries(
  ${PLATFORMS_NAME}
  PRIVATE
  COMPILER_FLAGS
  opengl32
  PUBLIC
  ${BASE_THIRD_PARTY_NAME}
)
target_include_directories(
  ${PLATFORMS_NAME}
  PRIVATE
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(
  ${PLATFORMS_NAME}
  PRIVATE
  HO_PROJECT_ROOT="${HO_PROJECT_ROOT}"
  HO_ASSET_ROOT="${HO_ASSET_ROOT}"
)

message(STATUS "[Module] Processing ${PLATFORMS_NAME} module finished.\n")
