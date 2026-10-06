#pragma once

/**
 * @file odometry_publisher.hpp
 * @brief Feeds the odometry produced by the control loop to whatever sits above it.
 *
 * The C++ master is the motor layer of a larger navigation stack - see
 * odometry_kinematics_summary.md, where GetRobotPose()/GetRobotTwist() are described as
 * feeding the navigation and ROS2 layers. Those layers want a steady stream at the
 * control rate (200 Hz by default), which is far too fast for a terminal, so the output
 * is offered as:
 *
 *   - an in-process callback, for anything linked into this binary;
 *   - a 64-byte binary datagram over UDP, for a subscriber in another process
 *     (a Python or ROS2 node) without having to rebuild this program.
 *
 * The wire format is fixed width and versioned (see params.hpp) so a subscriber can decode
 * it with six struct reads, and so a mismatched build shows up as an implausible pose
 * rather than silent corruption.
 *
 * Publishing is best effort by design: a subscriber that stalls or a socket that fills up
 * must never slow the 200 Hz control loop, so failures are counted and reported rather
 * than retried or propagated.
 */

#include "mbdv/params.hpp"

#include <sys/socket.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace mbdv {

class OdometryPublisher {
 public:
  using Callback = std::function<void(const OdometrySample&)>;

  OdometryPublisher() = default;
  ~OdometryPublisher();

  OdometryPublisher(const OdometryPublisher&) = delete;
  OdometryPublisher& operator=(const OdometryPublisher&) = delete;

  /// Applies the odometry section of @p params. Opens the UDP socket when asked to.
  /// @return false with @p error set only when UDP was requested and could not be opened.
  bool Configure(const Params& params, std::string* error);

  /// Registers the in-process subscriber. Passing an empty function disables it.
  void SetCallback(Callback cb) { callback_ = std::move(cb); }

  /**
   * @brief Delivers one sample. Called from the control loop, so it must be cheap.
   *
   * Honours odometry.udp_repeat by sending the sample that many times; a value above 1
   * makes a late-joining subscriber receive a steady stream instead of silence.
   */
  void Publish(const OdometrySample& sample);

  /// Closes the socket. Safe to call when nothing was ever opened.
  void Close();

  uint64_t SentSamples() const noexcept { return sent_.load(); }
  uint64_t DroppedSamples() const noexcept { return dropped_.load(); }
  const std::string& LastError() const noexcept { return last_error_; }

 private:
  bool SendUdp(const unsigned char* payload, std::size_t size);

  Callback callback_;
  int socket_{-1};
  struct sockaddr_storage dest_{};  ///< resolved UDP target
  std::size_t dest_len_{0};
  int repeat_{1};
  std::atomic<uint64_t> sent_{0};
  std::atomic<uint64_t> dropped_{0};
  std::string last_error_;
};

}  // namespace mbdv
