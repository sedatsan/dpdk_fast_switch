cmake_minimum_required(VERSION 3.14)
project(dpdk_fast_switch VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# MISRA / High Reliability Warning Flags
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DPDK REQUIRED libdpdk)

add_executable(${PROJECT_NAME} 
    src/main.c
    src/mac_table.c
    src/ipc.c
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${DPDK_INCLUDE_DIRS}
)
target_link_options(${PROJECT_NAME} PRIVATE ${DPDK_LDFLAGS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DPDK_LIBRARIES} pthread)
