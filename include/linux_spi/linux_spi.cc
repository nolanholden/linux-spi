#include "linux_spi.hpp"

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#include <linux/spi/spidev.h>

namespace linux_spi {

namespace sdata {
  int(*errlog_func)(const char* format, ...) = nullptr;
} // namespace sdata

void register_errno_logging(int(*log)(const char* format, ...)) {
  sdata::errlog_func = log;
}

#define LOG(format, ...) \
  do { \
      printf("[linux_spi:] " format "\n", ##__VA_ARGS__); \
  } while (0)
#define LOG_ERRNO() \
  do { \
    LOG("errno: %d\n", errno); \
  } while (0)

OpenFailure spi_open(spi_state& s) {
  s.set_fd(open(s.device(), O_RDWR));
  if (s.fd() <= 0) {
    LOG("failed to open SPI device at path %s, fd resolved as %d", s.device(), s.fd());
    LOG_ERRNO();
    return OpenFailure::UNABLE_OPEN_DEVICE_PATH;
  }
  if (ioctl(s.fd(), SPI_IOC_WR_MODE, &s.config().mode) < 0) {
    LOG("failed to set SPI mode");
    LOG_ERRNO();
    return OpenFailure::UNABLE_SET_MODE;
  }
  if (ioctl(s.fd(), SPI_IOC_RD_MODE, &s.config().mode) < 0) {
    LOG("failed to set SPI mode");
    LOG_ERRNO();
    return OpenFailure::UNABLE_SET_MODE;
  }
  if (ioctl(s.fd(), SPI_IOC_WR_BITS_PER_WORD, &s.config().bits_per_word) < 0) {
    LOG("failed to set SPI bits per word");
    LOG_ERRNO();
    return OpenFailure::UNABLE_SET_BPW;
  }
  if (ioctl(s.fd(), SPI_IOC_RD_BITS_PER_WORD, &s.config().bits_per_word) < 0) {
    LOG("failed to set SPI bits per word");
    LOG_ERRNO();
    return OpenFailure::UNABLE_SET_BPW;
  }
  if (ioctl(s.fd(), SPI_IOC_WR_MAX_SPEED_HZ, &s.config().speed) < 0) {
    LOG("failed to set SPI speed");
    LOG_ERRNO();
    return OpenFailure::UNABLE_SET_SPEED;
  }
  if (ioctl(s.fd(), SPI_IOC_RD_MAX_SPEED_HZ, &s.config().speed) < 0) {
    LOG("failed to set SPI speed");
    LOG_ERRNO();
    return OpenFailure::UNABLE_SET_SPEED;
  }

  return OpenFailure::NONE;
}

int spi_close(int fd) {
  auto res = close(fd);
  if (res < 0) {
    LOG("could not close");
    LOG_ERRNO();
  }

  return res;
}

int spi_xfer(const spi_state& state, uint8_t *tx_buffer, uint8_t tx_len, uint8_t *rx_buffer, uint8_t rx_len){
    struct spi_ioc_transfer spi_message{};
    spi_message.bits_per_word = state.config().bits_per_word;
    spi_message.delay_usecs = 0;
    spi_message.len = tx_len;
    spi_message.rx_buf = (unsigned long)rx_buffer;
    spi_message.speed_hz = state.config().speed;
    spi_message.tx_buf = (unsigned long)tx_buffer;

    memset(&spi_message, 0, sizeof(spi_message));
    
    auto res = ioctl(state.fd(), SPI_IOC_MESSAGE(2), &spi_message);
    if (res < 0) {
      LOG("could not tranfer");
      LOG_ERRNO();
    }

    return res;
}

} // namespace linux_spi
