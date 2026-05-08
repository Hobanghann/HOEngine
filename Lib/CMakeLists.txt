message(STATUS "[Module] Start to process ${THIRD_PARTY_NAME} module.")

if(DISABLE_DOWNLOAD_EXTERNAL_LIBRARIES)

# Add dummy library
add_library(${THIRD_PARTY_NAME} INTERFACE)

else()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
##########################################################
# Set external libraries name
##########################################################
set(ASSIMP_NAME assimp)
set(IMAGE_LOADER_NAME stb)
set(INI_PARSER_NAME inicpp)
set(GLAD_NAME glad)
set(IMGUI_NAME imgui)
set(IM_VIEW_GUIZMO_NAME ImViewGuizmo)
set(FONT_AWESOME_NAME icons_font_awesome7)
set(SPDLOG_NAME spdlog)

##########################################################
# Download and fetch external libraries
##########################################################

# Declare download targets

# Assimp
FetchContent_Declare(
    ${ASSIMP_NAME}
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.4
    GIT_SHALLOW ON
    )
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)

# stb
FetchContent_Declare(
    ${IMAGE_LOADER_NAME}
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW ON
    )

# ini parser
FetchContent_Declare(
    ${INI_PARSER_NAME}
    GIT_REPOSITORY https://github.com/Rookfighter/inifile-cpp.git
    GIT_TAG main
    GIT_SHALLOW ON
    )

# imgui
FetchContent_Declare(
    ${IMGUI_NAME}
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.7-docking
    GIT_SHALLOW ON
)

# ImViewGuizmo
FetchContent_Declare(
    ${IM_VIEW_GUIZMO_NAME}
    GIT_REPOSITORY https://github.com/Ka1serM/ImViewGuizmo
    GIT_TAG v1.0.2
    GIT_SHALLOW ON
)

# spdlog
FetchContent_Declare(
    ${SPDLOG_NAME}
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.17.0
    GIT_SHALLOW ON
)

# Download

message (STATUS "[Download] Start to Download " ${ASSIMP_NAME} "...")
FetchContent_MakeAvailable(${ASSIMP_NAME})
message (STATUS "[Download] Downloading " ${ASSIMP_NAME} " finished.")

message (STATUS "[Download] Start to Download " ${IMAGE_LOADER_NAME} "...")
FetchContent_MakeAvailable(${IMAGE_LOADER_NAME})
message (STATUS "[Download] Downloading " ${IMAGE_LOADER_NAME} " finished.")

message (STATUS "[Download] Start to Download " ${INI_PARSER_NAME} "...")
FetchContent_MakeAvailable(${INI_PARSER_NAME})
message (STATUS "[Download] Downloading " ${INI_PARSER_NAME} " finished")

message (STATUS "[Download] Start to Download " ${IMGUI_NAME} "...")
FetchContent_MakeAvailable(${IMGUI_NAME})
message (STATUS "[Download] Downloading " ${IMGUI_NAME} " finished.")

message (STATUS "[Download] Start to Download " ${IM_VIEW_GUIZMO_NAME} "...")
FetchContent_MakeAvailable(${IM_VIEW_GUIZMO_NAME})
message (STATUS "[Download] Downloading " ${IM_VIEW_GUIZMO_NAME} " finished.")

message (STATUS "[Download] Start to Download " ${SPDLOG_NAME} "...")
FetchContent_MakeAvailable(${SPDLOG_NAME})
message (STATUS "[Download] Downloading " ${SPDLOG_NAME} " finished.")

##########################################################
# Add target for raw external libraries
##########################################################
add_library(
    ${GLAD_NAME} 
    STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c)
target_include_directories(
    ${GLAD_NAME} 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/glad/include)

add_library(
    ${FONT_AWESOME_NAME} 
    INTERFACE
    )
target_include_directories(
    ${FONT_AWESOME_NAME} 
    INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

################################################################################
# Set library properties
################################################################################

# Assimp targets
function(get_all_targets_recursive targets_var dir)
    get_property(
        subdirectories 
        DIRECTORY ${dir} 
        PROPERTY SUBDIRECTORIES
        )
    foreach(subdir ${subdirectories})
        get_all_targets_recursive(${targets_var} ${subdir})
    endforeach()
    
    get_property(
        current_targets 
        DIRECTORY ${dir} 
        PROPERTY BUILDSYSTEM_TARGETS
        )
    
    list(APPEND ${targets_var} ${current_targets})
    set(${targets_var} ${${targets_var}} PARENT_SCOPE)
endfunction()

FetchContent_GetProperties(
    ${ASSIMP_NAME} 
    SOURCE_DIR 
    ASSIMP_DIR
    )

set(ALL_ASSIMP_TARGETS "")
if(ASSIMP_DIR)
    get_all_targets_recursive(ALL_ASSIMP_TARGETS ${ASSIMP_DIR})
endif()

foreach(target IN LISTS ALL_ASSIMP_TARGETS)
    set_target_properties(
        ${target} 
        PROPERTIES 
        FOLDER "ThirdParty/${ASSIMP_NAME}"
        SYSTEM TRUE
    )
endforeach()


# stb
add_library(
    ${IMAGE_LOADER_NAME} 
    STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/${IMAGE_LOADER_NAME}.cc
    )
target_include_directories(
    ${IMAGE_LOADER_NAME} 
    PRIVATE 
    ${${IMAGE_LOADER_NAME}_SOURCE_DIR}
    )    
set_target_properties(
    ${IMAGE_LOADER_NAME} 
    PROPERTIES 
    FOLDER "ThirdParty/${IMAGE_LOADER_NAME}"
    SYSTEM TRUE
    )
if (MSVC)
    target_compile_definitions(
        ${IMAGE_LOADER_NAME} 
        PRIVATE 
        _CRT_SECURE_NO_WARNINGS)
    target_compile_options(
        ${IMAGE_LOADER_NAME} 
        PRIVATE 
        /wd4996)
endif()
    

# ini parser
set_target_properties(${INI_PARSER_NAME} PROPERTIES 
    FOLDER "ThirdParty/${INI_PARSER_NAME}"
    SYSTEM TRUE
    )

# glad
set_target_properties(${GLAD_NAME} PROPERTIES 
    FOLDER "ThirdParty/${GLAD_NAME}"
    SYSTEM TRUE
    )

# imgui
FetchContent_GetProperties(${IMGUI_NAME})
if(NOT ${IMGUI_NAME}_POPULATED)
    FetchContent_Populate(${IMGUI_NAME})
endif()

file(GLOB IMGUI_CORE_SOURCES "${${IMGUI_NAME}_SOURCE_DIR}/*.cpp")
file(GLOB IMGUI_CORE_HEADERS "${${IMGUI_NAME}_SOURCE_DIR}/*.h")

file(GLOB IMGUI_BACKEND_SOURCES "${${IMGUI_NAME}_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp" "${${IMGUI_NAME}_SOURCE_DIR}/backends/imgui_impl_win32.cpp")
file(GLOB IMGUI_BACKEND_HEADERS "${${IMGUI_NAME}_SOURCE_DIR}/backends/imgui_impl_opengl3.h" "${${IMGUI_NAME}_SOURCE_DIR}/backends/imgui_impl_win32.h")

source_group(
    "core" 
    FILES 
    ${IMGUI_CORE_SOURCES} 
    ${IMGUI_CORE_HEADERS}
    )

source_group(
    "backends" 
    FILES 
    ${IMGUI_BACKEND_SOURCES} 
    ${IMGUI_BACKEND_HEADERS}
    )

add_library(
    ${IMGUI_NAME} 
    STATIC 
    ${IMGUI_CORE_SOURCES} 
    ${IMGUI_CORE_HEADERS}
    ${IMGUI_BACKEND_SOURCES} 
    ${IMGUI_BACKEND_HEADERS}
    )

target_include_directories(
    ${IMGUI_NAME} 
    PUBLIC 
    ${${IMGUI_NAME}_SOURCE_DIR}
    ${${IMGUI_NAME}_SOURCE_DIR}/backends
    )

set_target_properties(${IMGUI_NAME} PROPERTIES 
    FOLDER "ThirdParty/${IMGUI_NAME}"
    SYSTEM TRUE
    )

# ImViewGuizmo
add_library(
    ${IM_VIEW_GUIZMO_NAME} 
    INTERFACE
    )
target_include_directories(
    ${IM_VIEW_GUIZMO_NAME} 
    INTERFACE 
    ${${IM_VIEW_GUIZMO_NAME}_SOURCE_DIR}
    )

if (TARGET ${IM_VIEW_GUIZMO_NAME})
    set_target_properties(${IM_VIEW_GUIZMO_NAME} PROPERTIES 
        FOLDER "ThirdParty/${IM_VIEW_GUIZMO_NAME}"
        SYSTEM TRUE
    )
endif()

# font awesome
set_target_properties(${FONT_AWESOME_NAME} PROPERTIES 
    FOLDER "ThirdParty/${FONT_AWESOME_NAME}"
    SYSTEM TRUE
    )

# spdlog
set_target_properties(${SPDLOG_NAME} PROPERTIES 
    FOLDER "ThirdParty/${SPDLOG_NAME}"
    SYSTEM TRUE
    )

################################################################################
# Add library target
################################################################################
add_library(${THIRD_PARTY_NAME} INTERFACE)
target_link_libraries(
    ${THIRD_PARTY_NAME}
    INTERFACE
    ${ASSIMP_NAME}
    ${IMAGE_LOADER_NAME}
    ${INI_PARSER_NAME}
    ${GLAD_NAME}
    ${IMGUI_NAME}
    ${IM_VIEW_GUIZMO_NAME}
    ${FONT_AWESOME_NAME}
    ${SPDLOG_NAME}
    )
    
target_include_directories(
  ${THIRD_PARTY_NAME}
  SYSTEM 
  INTERFACE
  ${${ASSIMP_NAME}_SOURCE_DIR}/include
  ${${IMAGE_LOADER_NAME}_SOURCE_DIR}
  ${${INI_PARSER_NAME}_SOURCE_DIR}/include
  ${${GLAD_NAME}_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}
  )

endif()

message(STATUS "[Module] Processing ${THIRD_PARTY_NAME} finished.\n")
