#include "mbdv/odometry_publisher.hpp"

#include "mbdv/diagnostics.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <mutex>

namespace mbdv {

OdometryPublisher::~OdometryPublisher() { Close(); }

bool OdometryPublisher::Configure(const Params& params, std::string* error) {
  Close();
  repeat_ = params.udp_repeat < 1 ? 1 : params.udp_repeat;
  if (!params.publish_udp) return true;

  socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_ < 0) {
    if (error) {
      *error = std::string("odometry.udp is on but socket() failed: ") +
               std::strerror(errno);
    }
    return false;
  }

  // A multicast datagram that nobody joined would still fill the socket buffer at 200 Hz,
  // so the send buffer is enlarged rather than the sends being dropped silently later.
  int sndbuf = 512 * 1024;
  ::setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf));

  struct sockaddr_in addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(params.udp_port);
  if (::inet_pton(AF_INET, params.udp_host.c_str(), &addr.sin_addr) != 1) {
    if (error) {
      *error = "odometry.udp_host '" + params.udp_host + "' is not an IPv4 address";
    }
    Close();
    return false;
  }
  dest_ = {};
  std::memcpy(&dest_, &addr, sizeof(addr));
  dest_len_ = sizeof(addr);
  return true;
}

void OdometryPublisher::Close() {
  if (socket_ >= 0) {
    ::close(socket_);
    socket_ = -1;
  }
  dest_len_ = 0;
}

bool OdometryPublisher::SendUdp(const unsigned char* payload, std::size_t size) {
  if (socket_ < 0 || dest_len_ == 0) return false;
  // MSG_NOSIGNAL: a datagram send to a dead subscriber must not raise SIGPIPE, which would
  // otherwise take the whole control process down.
  const ssize_t n = ::sendto(socket_, payload, size, MSG_NOSIGNAL,
                             reinterpret_cast<const struct sockaddr*>(&dest_), dest_len_);
  return n == static_cast<ssize_t>(size);
}

void OdometryPublisher::Publish(const OdometrySample& sample) {
  if (callback_) callback_(sample);

  if (socket_ < 0) return;

  unsigned char payload[kOdometrySampleWireSize];
  if (!SerializeOdometrySample(sample, payload, sizeof(payload))) {
    ++dropped_;
    return;
  }
  for (int i = 0; i < repeat_; ++i) {
    if (SendUdp(payload, sizeof(payload))) {
      ++sent_;
    } else {
      ++dropped_;
      // Reported once per episode rather than per sample: at 200 Hz an unconditional log
      // would bury everything else.
      if (dropped_.load() == 1 || dropped_.load() % 200 == 0) {
        last_error_ = std::string("odometry UDP send failed: ") + std::strerror(errno);
        LogWarn(Stage::S14_MOTION_TRACKING, nullptr,
                Str("odometry UDP send failed (", static_cast<long long>(dropped_.load()),
                    " dropped so far): ", last_error_));
      }
    }
  }
}

}  // namespace mbdv
