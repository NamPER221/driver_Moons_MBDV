cmake_minimum_required(VERSION 3.16)
project(mbdv_servo_canopen LANGUAGES CXX)

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

set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)
set(SINGLE_CONFIG_DIR ${CONFIG_DIR}/single_axis_500k)

# 1. Custom rule for Dual-Axis (1 Mbps) DCF & BIN generation
add_custom_command(
    OUTPUT ${CONFIG_DIR}/master.dcf
           ${CONFIG_DIR}/slave_1.bin
           ${CONFIG_DIR}/slave_2.bin
    COMMAND dcfgen -r -d ${CONFIG_DIR} ${CONFIG_DIR}/master.yaml
    DEPENDS ${CONFIG_DIR}/master.yaml
            ${CONFIG_DIR}/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling Dual-Axis CANopen Master DCF and slave configs (1 Mbps)"
)

add_custom_target(generate_canopen_dcf ALL
    DEPENDS ${CONFIG_DIR}/master.dcf
            ${CONFIG_DIR}/slave_1.bin
            ${CONFIG_DIR}/slave_2.bin
)

# 2. Custom rule for Single-Axis 1 (500 kbps) DCF & BIN generation
add_custom_command(
    OUTPUT ${SINGLE_CONFIG_DIR}/master.dcf
           ${SINGLE_CONFIG_DIR}/slave_1.bin
    COMMAND dcfgen -r -d ${SINGLE_CONFIG_DIR} ${SINGLE_CONFIG_DIR}/master.yaml
    DEPENDS ${SINGLE_CONFIG_DIR}/master.yaml
            ${SINGLE_CONFIG_DIR}/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Compiling Single-Axis 1 CANopen Master DCF (500 kbps, Node 1)"
)

add_custom_target(generate_single_axis_500k_dcf ALL
    DEPENDS ${SINGLE_CONFIG_DIR}/master.dcf
            ${SINGLE_CONFIG_DIR}/slave_1.bin
)

# Target 1: Dual-Axis Controller Executable (1 Mbps)
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

# Target 2: Single-Axis 1 Controller Executable (500 kbps, Node ID 1)
add_executable(mbdv_single_axis_node
    src/main_single_axis.cpp
    src/mbdv_axis_driver.cpp
    src/single_axis_controller.cpp
)
add_dependencies(mbdv_single_axis_node generate_single_axis_500k_dcf)
target_link_libraries(mbdv_single_axis_node PRIVATE
    ${LELY_LIBRARIES}
    pthread
    rt
)

# Install rules
install(TARGETS mbdv_dual_axis_node mbdv_single_axis_node DESTINATION bin)
install(DIRECTORY ${CONFIG_DIR} DESTINATION share/mbdv_canopen)
