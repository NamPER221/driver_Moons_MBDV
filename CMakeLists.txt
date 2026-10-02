cmake_minimum_required(VERSION 3.16)
project(mbdv_dual_axis_canopen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Find lely-core libraries via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(LELY REQUIRED
    liblely-coapp
    liblely-io2
    liblely-ev
    liblely-can
    liblely-co
    liblely-util
    liblely-libc
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LELY_INCLUDE_DIRS}
)

# Custom rule to generate master.dcf and concise binary DCFs from master.yaml
set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)
add_custom_command(
    OUTPUT ${CONFIG_DIR}/master.dcf
           ${CONFIG_DIR}/slave_1.bin
           ${CONFIG_DIR}/slave_2.bin
    COMMAND dcfgen -r -d ${CONFIG_DIR} ${CONFIG_DIR}/master.yaml
    DEPENDS ${CONFIG_DIR}/master.yaml
            ${CONFIG_DIR}/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling CANopen Master DCF and slave concise configs via dcfgen"
)

add_custom_target(generate_canopen_dcf ALL
    DEPENDS ${CONFIG_DIR}/master.dcf
            ${CONFIG_DIR}/slave_1.bin
            ${CONFIG_DIR}/slave_2.bin
)

# Executable target
add_executable(mbdv_dual_axis_node
    src/main.cpp
    src/mbdv_axis_driver.cpp
    src/dual_axis_controller.cpp
    src/diff_drive_kinematics.cpp
)

add_dependencies(mbdv_dual_axis_node generate_canopen_dcf)

target_link_libraries(mbdv_dual_axis_node PRIVATE
    ${LELY_LIBRARIES}
    pthread
    rt
)

# Install rules
install(TARGETS mbdv_dual_axis_node DESTINATION bin)
install(DIRECTORY ${CONFIG_DIR} DESTINATION share/mbdv_dual_axis)
