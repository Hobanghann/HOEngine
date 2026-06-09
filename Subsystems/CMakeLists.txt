message(STATUS "[Module] Start to process ${SUBSYSTEMS_NAME} module.")

################################################################################
# Read source files
################################################################################
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc ${CMAKE_CURRENT_SOURCE_DIR}/*.vert ${CMAKE_CURRENT_SOURCE_DIR}/*.frag)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

set(ALL_FILES ${SOURCES} ${HEADERS})

################################################################################
# Repackiging source files
################################################################################
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_FILES})

################################################################################
# Add library target
################################################################################
add_library(${SUBSYSTEMS_NAME} STATIC ${ALL_FILES})

################################################################################
# Set target properties
################################################################################
target_link_libraries(
    ${SUBSYSTEMS_NAME}
    PRIVATE
    COMPILER_FLAGS
    PUBLIC
    ${CORE_NAME}
    ${APP_THIRD_PARTY_NAME}
    )
target_include_directories(
    ${SUBSYSTEMS_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

message(STATUS "[Module] Processing ${SUBSYSTEMS_NAME} module finished.\n")
