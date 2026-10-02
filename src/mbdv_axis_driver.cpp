#include "mbdv/mbdv_axis_driver.hpp"

#include <future>
#include <iomanip>
#include <iostream>

namespace mbdv {

MbdvAxisDriver::MbdvAxisDriver(lely::canopen::AsyncMaster& master,
                               uint8_t node_id, std::string axis_name)
    : lely::canopen::FiberDriver(master, node_id),
      node_id_(node_id),
      axis_name_(std::move(axis_name)) {}

void MbdvAxisDriver::RequestNmtStart() {
  std::cout << "[" << axis_name_ << " | Node " << static_cast<int>(node_id_)
            << "] Sending NMT START (0x01) command..." << std::endl;
  master.Command(lely::canopen::NmtCommand::START, id());
}

void MbdvAxisDriver::RequestNmtPreOp() {
  master.Command(lely::canopen::NmtCommand::ENTER_PREOP, id());
}

void MbdvAxisDriver::RequestNmtReset() {
  master.Command(lely::canopen::NmtCommand::RESET_NODE, id());
}

void MbdvAxisDriver::SetModeOfOperation(CiA402Mode mode) {
  Defer([this, mode]() {
    std::cout << "[" << axis_name_ << "] Setting Mode of Operation to "
              << cia402_mode_to_string(mode) << " (" << static_cast<int>(mode)
              << ")..." << std::endl;
    // 1. Write via SDO
    try {
      int8_t mode_val = static_cast<int8_t>(mode);
      Wait(AsyncWrite(0x6060, 0, mode_val));
    } catch (const std::exception& ex) {
      std::cerr << "[" << axis_name_ << "] Warning: SDO SetMode failed ("
                << ex.what() << ")" << std::endl;
    }
    // 2. Write to RPDO1 buffer (if mapped)
    try {
      tpdo_mapped[0x6060][0] = static_cast<int8_t>(mode);
      master.TpdoEvent();
    } catch (...) {
      // Continue if not in active TPDO
    }
  });
}

void MbdvAxisDriver::SendControlword(uint16_t controlword) {
  Defer([this, controlword]() {
    tpdo_mapped[0x6040][0] = controlword;
    master.TpdoEvent();
  });
}

void MbdvAxisDriver::ResetFault() {
  Defer([this]() {
    std::cout << "[" << axis_name_ << "] Resetting Fault (CW = 0x0080)..."
              << std::endl;
    tpdo_mapped[0x6040][0] = controlword_commands::FAULT_RESET;
    master.TpdoEvent();
    USleep(50000);  // 50 ms pulse
    tpdo_mapped[0x6040][0] = controlword_commands::DISABLE_VOLTAGE;
    master.TpdoEvent();
    USleep(20000);
  });
}

void MbdvAxisDriver::QuickStop() {
  Defer([this]() {
    tpdo_mapped[0x6040][0] = controlword_commands::QUICK_STOP;
    master.TpdoEvent();
  });
}

bool MbdvAxisDriver::DisableServo() {
  auto prom = std::make_shared<std::promise<bool>>();
  auto fut = prom->get_future();

  Defer([this, prom]() {
    std::cout << "[" << axis_name_ << "] Disabling Servo (CW = 0x0007)..."
              << std::endl;
    tpdo_mapped[0x6040][0] = controlword_commands::DISABLE_OPERATION;
    master.TpdoEvent();
    bool ok = WaitForState(CiA402State::SWITCHED_ON, std::chrono::milliseconds(1000));
    prom->set_value(ok);
  });

  if (fut.wait_for(std::chrono::milliseconds(1500)) == std::future_status::ready) {
    return fut.get();
  }
  return false;
}

bool MbdvAxisDriver::EnableServo(std::chrono::milliseconds timeout) {
  auto prom = std::make_shared<std::promise<bool>>();
  auto fut = prom->get_future();

  Defer([this, prom, timeout]() {
    std::cout << "[" << axis_name_ << "] Starting CiA 402 Servo ON Sequence..."
              << std::endl;

    // Recover from Fault if present
    if (GetCiA402State() == CiA402State::FAULT) {
      std::cout << "[" << axis_name_ << "] Drive in FAULT, executing Fault Reset..."
                << std::endl;
      tpdo_mapped[0x6040][0] = controlword_commands::FAULT_RESET;
      master.TpdoEvent();
      USleep(50000);
      tpdo_mapped[0x6040][0] = controlword_commands::DISABLE_VOLTAGE;
      master.TpdoEvent();
      USleep(50000);
    }

    // Step 1: Send 0x0006 (Shutdown) -> Wait for READY_TO_SWITCH_ON
    std::cout << "[" << axis_name_ << "] Step 1: Sending Shutdown (0x0006)..."
              << std::endl;
    tpdo_mapped[0x6040][0] = controlword_commands::SHUTDOWN;
    master.TpdoEvent();
    if (!WaitForState(CiA402State::READY_TO_SWITCH_ON, timeout)) {
      std::cerr << "[" << axis_name_
                << "] Timeout waiting for READY_TO_SWITCH_ON (Current: "
                << cia402_state_to_string(GetCiA402State()) << ")" << std::endl;
      prom->set_value(false);
      return;
    }

    // Step 2: Send 0x0007 (Switch ON) -> Wait for SWITCHED_ON
    std::cout << "[" << axis_name_ << "] Step 2: Sending Switch ON (0x0007)..."
              << std::endl;
    tpdo_mapped[0x6040][0] = controlword_commands::SWITCH_ON;
    master.TpdoEvent();
    if (!WaitForState(CiA402State::SWITCHED_ON, timeout)) {
      std::cerr << "[" << axis_name_
                << "] Timeout waiting for SWITCHED_ON (Current: "
                << cia402_state_to_string(GetCiA402State()) << ")" << std::endl;
      prom->set_value(false);
      return;
    }

    // Step 3: Send 0x000F (Enable Operation) -> Wait for OPERATION_ENABLED
    std::cout << "[" << axis_name_
              << "] Step 3: Sending Enable Operation / Servo ON (0x000F)..."
              << std::endl;
    tpdo_mapped[0x6040][0] = controlword_commands::ENABLE_OPERATION;
    master.TpdoEvent();
    if (!WaitForState(CiA402State::OPERATION_ENABLED, timeout)) {
      std::cerr << "[" << axis_name_
                << "] Timeout waiting for OPERATION_ENABLED (Current: "
                << cia402_state_to_string(GetCiA402State()) << ")" << std::endl;
      prom->set_value(false);
      return;
    }

    std::cout << "[" << axis_name_
              << "] Servo ON SUCCESS: Operation Enabled (State: 0x0027)!"
              << std::endl;
    prom->set_value(true);
  });

  if (fut.wait_for(timeout + std::chrono::milliseconds(1000)) == std::future_status::ready) {
    return fut.get();
  }
  return false;
}

void MbdvAxisDriver::SetTargetPosition(int32_t target_position,
                                      bool new_setpoint, bool immediate,
                                      bool relative) {
  Defer([this, target_position, new_setpoint, immediate, relative]() {
    // Write Target Position (0x607A) into RPDO2 buffer
    tpdo_mapped[0x607A][0] = target_position;

    uint16_t cw = controlword_commands::ENABLE_OPERATION;
    if (new_setpoint) {
      cw |= controlword_bits::NEW_SET_POINT;
    }
    if (immediate) {
      cw |= controlword_bits::CHANGE_SET_IMMEDIATELY;
    }
    if (relative) {
      cw |= controlword_bits::ABS_REL;
    }

    tpdo_mapped[0x6040][0] = cw;
    master.TpdoEvent();

    if (new_setpoint) {
      // Hold bit 4 (New Setpoint) briefly to ensure drive registers the rising edge
      USleep(20000);  // 20 ms
      tpdo_mapped[0x6040][0] = controlword_commands::ENABLE_OPERATION;
      master.TpdoEvent();
    }
  });
}

void MbdvAxisDriver::SetTargetVelocity(int32_t target_velocity) {
  Defer([this, target_velocity]() {
    tpdo_mapped[0x60FF][0] = target_velocity;
    tpdo_mapped[0x6040][0] = controlword_commands::ENABLE_OPERATION;
    master.TpdoEvent();
  });
}

bool MbdvAxisDriver::WaitForState(CiA402State target_state,
                                  std::chrono::milliseconds timeout) {
  auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start < timeout) {
    if (GetCiA402State() == target_state) {
      return true;
    }
    USleep(10000);  // 10 ms sleep in fiber
  }
  return false;
}

void MbdvAxisDriver::OnBoot(lely::canopen::NmtState /*st*/, char es,
                            const std::string& what) noexcept {
  if (es) {
    std::cerr << "[" << axis_name_ << " | Node " << static_cast<int>(node_id_)
              << "] Boot ERROR: " << what << " (code " << static_cast<int>(es)
              << ")" << std::endl;
  } else {
    std::cout << "[" << axis_name_ << " | Node " << static_cast<int>(node_id_)
              << "] Boot SUCCESS! Node configured and ready." << std::endl;
  }
}

void MbdvAxisDriver::OnState(lely::canopen::NmtState st) noexcept {
  bool is_op = (st == lely::canopen::NmtState::START);
  is_operational_.store(is_op);

  std::cout << "[" << axis_name_ << " | Node " << static_cast<int>(node_id_)
            << "] NMT state changed: "
            << (is_op ? "OPERATIONAL (0x05)"
                      : (st == lely::canopen::NmtState::PREOP ? "PRE-OPERATIONAL (0x7F)"
                                                              : "STOPPED (0x04)"))
            << std::endl;
}

void MbdvAxisDriver::OnRpdoWrite(uint16_t idx, uint8_t subidx) noexcept {
  if (idx == 0x6041 && subidx == 0) {
    uint16_t sw = rpdo_mapped[0x6041][0];
    statusword_.store(sw);
    state_.store(decode_cia402_state(sw));
    target_reached_.store((sw & statusword_bits::TARGET_REACHED) != 0);
  } else if (idx == 0x6064 && subidx == 0) {
    int32_t pos = rpdo_mapped[0x6064][0];
    actual_position_.store(pos);
  } else if (idx == 0x606C && subidx == 0) {
    int32_t vel = rpdo_mapped[0x606C][0];
    actual_velocity_.store(vel);
  }
}

void MbdvAxisDriver::OnEmcy(uint16_t eec, uint8_t er,
                            uint8_t msef[5]) noexcept {
  std::cerr << "[" << axis_name_ << " | Node " << static_cast<int>(node_id_)
            << "] CANopen EMERGENCY frame! ErrorCode: 0x"
            << std::hex << std::setw(4) << std::setfill('0') << eec
            << ", ErrorReg: 0x" << std::setw(2) << static_cast<int>(er)
            << ", Manufacturer Data: ["
            << static_cast<int>(msef[0]) << " " << static_cast<int>(msef[1]) << " "
            << static_cast<int>(msef[2]) << " " << static_cast<int>(msef[3]) << " "
            << static_cast<int>(msef[4]) << "]" << std::dec << std::endl;
}

}  // namespace mbdv
