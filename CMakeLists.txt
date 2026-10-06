cmake_minimum_required(VERSION 3.16)
project(mbdv_servo_canopen LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Used by the post-generation DCF repair step (tools/fix_master_dcf.py).
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Add compiler warnings
add_compile_options(-Wall -Wextra -Wpedantic)

# Find lely-core libraries via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAMLCPP REQUIRED yaml-cpp)
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
    ${YAMLCPP_INCLUDE_DIRS}
)

set(CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config)

# DCF & BIN generation for both axes of one drive (500 kbps, nodes 1 and 2).
#
# dcfgen describes a *slave*, so the master DCF it emits points 0x1400..0x1800..
# at local mirror objects (0x2001/0x2002/...) instead of the real CiA 402 indices
# (0x6040/0x607A/0x60FF/0x6041/...), and never emits those objects at all.
# lely builds a master's PDO transmit/receive tables from the master's own object
# dictionary, so with that DCF co_dev_tpdo_event() rejects every write and *no RPDO
# frame is ever put on the bus* while SDO keeps working - the drive then never
# enables and never moves. See tools/fix_master_dcf.py, which rewrites the PDO
# records and refuses to write unless every mapping resolves.
add_custom_command(
    OUTPUT ${CONFIG_DIR}/master.dcf
           ${CONFIG_DIR}/slave_1.bin
           ${CONFIG_DIR}/slave_2.bin
    COMMAND dcfgen -r -d ${CONFIG_DIR} ${CONFIG_DIR}/master.yaml
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/fix_master_dcf.py
            ${CONFIG_DIR}/master.dcf --nodes 1,2
    DEPENDS ${CONFIG_DIR}/master.yaml
            ${CONFIG_DIR}/CANOPEN-EDS-MBDV-Servo-DulAxes-V1.0.eds
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/fix_master_dcf.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating Dual-Axis CANopen Master DCF and slave configs (500 kbps)"
)

add_custom_target(generate_canopen_dcf ALL
    DEPENDS ${CONFIG_DIR}/master.dcf
            ${CONFIG_DIR}/slave_1.bin
            ${CONFIG_DIR}/slave_2.bin
)

# Absolute project root baked into the binaries so the tools can locate the generated
# master.dcf no matter which directory they are launched from.
add_compile_definitions(MBDV_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(mbdv_dual_axis_node
    src/main.cpp
    src/params.cpp
    src/odometry_publisher.cpp
    src/mbdv_axis_driver.cpp
    src/dual_axis_controller.cpp
    src/can_sniffer.cpp
    src/config_path.cpp
    src/diagnostics.cpp
    src/drive_errors.cpp
    src/diff_drive_kinematics.cpp
)
add_dependencies(mbdv_dual_axis_node generate_canopen_dcf)
target_link_libraries(mbdv_dual_axis_node PRIVATE
    ${LELY_LIBRARIES}
    ${YAMLCPP_LIBRARIES}
    pthread
    rt
)

# Install rules
install(TARGETS mbdv_dual_axis_node DESTINATION bin)
install(DIRECTORY ${CONFIG_DIR} DESTINATION share/mbdv_canopen)
