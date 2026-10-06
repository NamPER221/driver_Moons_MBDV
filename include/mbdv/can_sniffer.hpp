#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <thread>

namespace mbdv {

/**
 * @brief A second, receive-only SocketCAN socket that mirrors every frame on the bus.
 *
 * Why this exists. lely keeps ONE object dictionary per master, so a TPDO from node 1 and
 * a TPDO from node 2 that map the same CiA 402 object land in the same place and overwrite
 * each other. Observed on this bus: node 1 reported no feedback at all, node 2 reported
 * hundreds, and node 2's velocity read back a mixture of both axes' setpoints - which
 * blocks `AllAxesAlive()`, and therefore blocks the differential-drive teleop from
 * commanding anything. Pointing the TPDO mapping at per-node shadow indices inside the
 * master's OD does not help; lely then stops resolving either node's mapping.
 *
 * A second socket sidesteps the whole problem: Linux delivers a copy of each frame to every
 * CAN_RAW socket bound to the interface, so lely keeps its own socket for SDO/NMT/heartbeat
 * and transmit, and this one decodes the drive's TPDO payloads from raw bytes, per node.
 * That is exactly what the known-good bring-up on this hardware does.
 *
 * Frames are handed to the callback on the sniffer's own thread, so the callback must be
 * safe to call from a thread other than the control loop.
 */
class CanSniffer {
 public:
  /// Called for every data frame; @p id is the 11-bit CAN identifier.
  using FrameHandler =
      std::function<void(uint32_t id, const uint8_t* data, uint8_t len)>;

  CanSniffer() = default;
  ~CanSniffer();

  CanSniffer(const CanSniffer&) = delete;
  CanSniffer& operator=(const CanSniffer&) = delete;

  /**
   * @brief Opens the socket and starts the reader thread.
   * @return false with @p error set when the interface is missing or the socket fails.
   */
  bool Start(const std::string& interface, FrameHandler handler, std::string* error);

  /// Stops the reader thread and closes the socket. Safe to call twice.
  void Stop();

  /// Frames handed to the callback since Start(), for diagnostics.
  uint64_t FrameCount() const noexcept { return frame_count_.load(); }

  /// Frames dropped because the reader could not keep up, or on a socket error.
  uint64_t ErrorCount() const noexcept { return error_count_.load(); }

 private:
  void Run(std::string interface);

  int fd_{-1};
  std::thread thread_;
  std::atomic<bool> running_{false};
  std::atomic<uint64_t> frame_count_{0};
  std::atomic<uint64_t> error_count_{0};
  FrameHandler handler_;
};

}  // namespace mbdv