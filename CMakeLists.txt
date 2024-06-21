cmake_minimum_required(VERSION 3.16)

project(find_mods LANGUAGES C)

include(GNUInstallDirs)

set(PROJECT_SCRIPT_DIR ${CMAKE_SOURCE_DIR}/scripts)
set(AUTORUN_SOURCE_PATH ${PROJECT_SCRIPT_DIR}/autorun.sh)
set(AUTORUN_TARGET_PATH ${CMAKE_BINARY_DIR}/auto_runhalo)

# Enable AddressSanitizer for Debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Enabling AddressSanitizer for Debug build")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

add_executable(${PROJECT_NAME} src/main.c)

find_package(PkgConfig REQUIRED)
if (PkgConfig_FOUND)
    pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)
    if (LIBXML2_FOUND)
        target_include_directories(${PROJECT_NAME} PUBLIC ${LIBXML2_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} ${LIBXML2_STATIC_LIBRARIES})
    else()
        message(FATAL_ERROR "libxml2 not found. please install libxml2 development package.")
    endif()
else()
    message(FATAL_ERROR "PkgConfig not found. please install pkg-config package.")
endif()

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${AUTORUN_SOURCE_PATH} ${AUTORUN_TARGET_PATH}
)

install(PROGRAMS ${AUTORUN_TARGET_PATH} 
    DESTINATION ${CMAKE_INSTALL_BINDIR} 
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
                GROUP_READ GROUP_EXECUTE 
                WORLD_READ WORLD_EXECUTE
)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
