message(STATUS "[Module] Start to process ${CORE_NAME} module.")
################################################################################
# define variables
################################################################################
set(HO_PROJECT_ROOT "${CMAKE_SOURCE_DIR}")
set(HO_ASSET_ROOT "${CMAKE_SOURCE_DIR}/Assets")

################################################################################
# Read source files
################################################################################
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(ALL_FILES ${SOURCES} ${HEADERS})

################################################################################
# Repackiging source files
################################################################################
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_FILES})

################################################################################
# Add library target
################################################################################
add_library(
    ${CORE_NAME}
    STATIC
    ${ALL_FILES})

################################################################################
# Set target properties
################################################################################
target_link_libraries(
    ${CORE_NAME}
    PRIVATE
    COMPILER_FLAGS
    PUBLIC
    ${BASE_THIRD_PARTY_NAME}
    )
target_include_directories(
    ${CORE_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )
target_compile_definitions(
  ${CORE_NAME}
  PRIVATE
  HO_PROJECT_ROOT="${HO_PROJECT_ROOT}"
  HO_ASSET_ROOT="${HO_ASSET_ROOT}"
)

message(STATUS "[Module] Processing ${CORE_NAME} module finished.\n")
