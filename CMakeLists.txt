cmake_minimum_required(VERSION 3.16)
project(mbdv_servo_canopen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Used by the post-generation DCF repair step (tools/fix_dcf_pdo_cobid.py).
find_package(Python3 COMPONENTS Interpreter REQUIRED)

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

# 1. Dual-axis DCF & BIN generation.
#
# dcfgen 2.3.5 writes the PDO COB-ID default values of 0x14xx:01 and 0x18xx:01 swapped in
# the generated DCF (reproducible even with no PDO configured in the YAML). The SDO
# download list it emits at the same time is correct, so the drive ends up configured
# properly - but lely parses the DCF text to build its own PDO tables, so the master
# would transmit RPDO on a TPDO COB-ID and listen for TPDO on an RPDO COB-ID. Symptom:
# SDO works while PDO fails in both directions. See tools/fix_dcf_pdo_cobid.py.
add_custom_command(
    OUTPUT ${CONFIG_DIR}/master.dcf
           ${CONFIG_DIR}/slave_1.bin
           ${CONFIG_DIR}/slave_2.bin
    COMMAND dcfgen -r -d ${CONFIG_DIR} ${CONFIG_DIR}/master.yaml
    DEPENDS ${CONFIG_DIR}/master.yaml
            ${CONFIG_DIR}/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating Dual-Axis CANopen Master DCF and slave configs (500 kbps)"
)

add_custom_target(generate_canopen_dcf ALL
    DEPENDS ${CONFIG_DIR}/master.dcf
            ${CONFIG_DIR}/slave_1.bin
            ${CONFIG_DIR}/slave_2.bin
)

# 2. Single-axis DCF & BIN generation (500 kbps, Node 1). Same DCF repair as above.
add_custom_command(
    OUTPUT ${SINGLE_CONFIG_DIR}/master.dcf
           ${SINGLE_CONFIG_DIR}/slave_1.bin
    COMMAND dcfgen -r -d ${SINGLE_CONFIG_DIR} ${SINGLE_CONFIG_DIR}/master.yaml
    DEPENDS ${SINGLE_CONFIG_DIR}/master.yaml
            ${SINGLE_CONFIG_DIR}/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating Single-Axis CANopen Master DCF (500 kbps, Node 1)"
)

add_custom_target(generate_single_axis_500k_dcf ALL
    DEPENDS ${SINGLE_CONFIG_DIR}/master.dcf
            ${SINGLE_CONFIG_DIR}/slave_1.bin
)

# Absolute project root baked into the binaries so the tools can locate the generated
# master.dcf no matter which directory they are launched from.
add_compile_definitions(MBDV_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Target 1: Dual-Axis Controller Executable
add_executable(mbdv_dual_axis_node
    src/main.cpp
    src/mbdv_axis_driver.cpp
    src/dual_axis_controller.cpp
    src/config_path.cpp
    src/diagnostics.cpp
    src/drive_errors.cpp
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
    src/config_path.cpp
    src/diagnostics.cpp
    src/drive_errors.cpp
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
