#ifndef _LINUXSPI_LINUXSPI_HPP_
#define _LINUXSPI_LINUXSPI_HPP_

#include <cstdint>
#include <utility>

namespace linux_spi {

struct spi_config {
  // "SPI mode," indicating clock polarity and clock phase (CPOL, CPHA)
	std::uint8_t mode;

  // SPI bits per word
  std::uint8_t bits_per_word;

  // SPI clock speed (Hz)
  std::uint32_t speed;
};

class spi_state {
 public:
  spi_state(const char* device_path, spi_config&& c)
    : device_(std::forward<decltype(device_)>(device_path))
    , config_(std::forward<decltype(config_)>(c)) {}

  const spi_config& config() { return config_; }

  const char* device() { return device_; }

  // Return the current file descriptor.
  int fd() { return fd_; }
  void set_fd(int fd) { fd_ = fd; }

 private:
  const spi_config config_;

  const char* const device_;

  // the current file descriptor
  int fd_ = 0;
};

enum class OpenFailure {
  NONE,
  UNABLE_OPEN_DEVICE_PATH,
  UNABLE_SET_BPW,
  UNABLE_SET_SPEED,
  UNABLE_SET_MODE,
};

OpenFailure spi_open(spi_state& s);
int spi_close(int fd);
int spi_xfer(int fd, uint8_t *tx_buffer, uint8_t tx_len, uint8_t *rx_buffer, uint8_t rx_len);

void register_errno_logging(int(*log)(const char* format, ...));

} // namespace linux_spi

#endif // _LINUXSPI_LINUXSPI_HPP_
