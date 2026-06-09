message(STATUS "[Module] Start to process ${APP_THIRD_PARTY_NAME} module.")

if(DISABLE_DOWNLOAD_EXTERNAL_LIBRARIES)

# Add dummy library
add_library(${APP_THIRD_PARTY_NAME} INTERFACE)

else()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
################################################################################
# Set external libraries name
################################################################################

################################################################################
# Download and fetch external libraries
################################################################################

# Declare download targets

# Download

################################################################################
# Add target for raw external libraries
################################################################################

################################################################################
# Set library properties
################################################################################

################################################################################
# Add BaseThirdPartyImpl library
################################################################################
set(APP_THIRD_PARTY_IMPLEMENTATION_NAME AppThirdPartyImpl)

add_library(
    ${APP_THIRD_PARTY_IMPLEMENTATION_NAME}
    STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${APP_THIRD_PARTY_IMPLEMENTATION_NAME}.cc
    )

target_link_libraries(
    ${APP_THIRD_PARTY_IMPLEMENTATION_NAME}
    PRIVATE
    ${BASE_THIRD_PARTY_NAME}
    ${CORE_NAME}
    )

set_target_properties(
    ${APP_THIRD_PARTY_IMPLEMENTATION_NAME} PROPERTIES
    FOLDER "ThirdParty/${APP_THIRD_PARTY_NAME}/Implementation"
    SYSTEM TRUE
    )

################################################################################
# Add library target
################################################################################
add_library(${APP_THIRD_PARTY_NAME} INTERFACE)
target_link_libraries(
    ${APP_THIRD_PARTY_NAME}
    INTERFACE
    ${APP_THIRD_PARTY_IMPLEMENTATION_NAME}
    )

target_include_directories(
  ${APP_THIRD_PARTY_NAME}
  SYSTEM
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

endif()

message(STATUS "[Module] Processing ${APP_THIRD_PARTY_NAME} finished.\n")
