message(STATUS "[Module] Start to process ${ENGINE_APPLICATION_NAME} module.")

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
add_library(${ENGINE_APPLICATION_NAME} STATIC ${ALL_FILES})

################################################################################
# Set target properties
################################################################################
target_link_libraries(
    ${ENGINE_APPLICATION_NAME}
    PRIVATE
    COMPILER_FLAGS
    PUBLIC
    ${SUBSYSTEMS_NAME}
    )
    
target_include_directories(
    ${ENGINE_APPLICATION_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

message(STATUS "[Module] Processing ${ENGINE_APPLICATION_NAME} module finished.\n")
