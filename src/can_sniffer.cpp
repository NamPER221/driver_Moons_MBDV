#include "mbdv/can_sniffer.hpp"

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace mbdv {

CanSniffer::~CanSniffer() { Stop(); }

bool CanSniffer::Start(const std::string& interface, FrameHandler handler,
                       std::string* error) {
  const auto fail = [error](const std::string& what) {
    if (error) *error = what;
    return false;
  };

  if (running_.load()) return fail("can sniffer already running");
  if (!handler) return fail("can sniffer needs a frame handler");

  ifreq ifr;
  std::memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_name[0] = '\0';
  std::strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);

  fd_ = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (fd_ < 0) return fail("socket(PF_CAN, SOCK_RAW): " + std::string(std::strerror(errno)));
  if (::ioctl(fd_, SIOCGIFINDEX, &ifr) < 0) {
    const std::string why = std::strerror(errno);
    ::close(fd_);
    fd_ = -1;
    return fail("SIOCGIFINDEX on '" + interface + "': " + why);
  }

  sockaddr_can addr;
  std::memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;
  if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    const std::string why = std::strerror(errno);
    ::close(fd_);
    fd_ = -1;
    return fail("bind('" + interface + "'): " + why + " (needs CAP_NET_RAW/root)");
  }

  // No filters and no error frames: this socket only mirrors the data frames lely already
  // consumes, so leaving error reporting on would flood it with bus-error noise.
  const int zero = 0;
  ::setsockopt(fd_, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &zero, sizeof(zero));
  ::setsockopt(fd_, SOL_CAN_RAW, CAN_RAW_FILTER, &zero, sizeof(zero));

  handler_ = std::move(handler);
  frame_count_.store(0);
  error_count_.store(0);
  running_.store(true);
  thread_ = std::thread(&CanSniffer::Run, this, interface);
  return true;
}

void CanSniffer::Stop() {
  running_.store(false);
  if (thread_.joinable()) thread_.join();
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

void CanSniffer::Run(std::string interface) {
  (void)interface;
  const int fd = fd_;
  while (running_.load()) {
    pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    const int ready = ::poll(&pfd, 1, 200);
    if (ready < 0) {
      if (errno == EINTR) continue;
      ++error_count_;
      break;
    }
    if (ready == 0) continue;

    can_frame frame;
    const ssize_t n = ::read(fd, &frame, sizeof(frame));
    if (n != static_cast<ssize_t>(sizeof(frame))) {
      if (n < 0 && (errno == EAGAIN || errno == EINTR)) continue;
      ++error_count_;
      continue;
    }
    if (frame.can_id & CAN_ERR_FLAG) continue;
    const uint32_t id = frame.can_id & (frame.can_id & CAN_EFF_FLAG ? 0x1FFFFFFF : 0x7FF);
    if (handler_) handler_(id, frame.data, frame.can_dlc);
    ++frame_count_;
  }
}

}  // namespace mbdv