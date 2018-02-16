#include "linux_spi.hpp"

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

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
  int res = 0;
	int           fd;
	int           blocksize   =  tx_len;
	int           blocknumber = -1;
	int           offset      =  0;
	int           nb          =  0;
	int           speed       = state.config().speed;
	int           orig_speed  = -1;


  tx_buffer[0] = 0x01;
  tx_buffer[1] = 0x36;

	struct spi_ioc_transfer transfer = {
		.tx_buf        = 0,
		.rx_buf        = 0,
		.len           = 0,
		.delay_usecs   = 0,
		.speed_hz      = 0,
		.bits_per_word = 0,
	};

	memset(rx_buffer, 0, blocksize);
	memset(tx_buffer, 0, blocksize);

	transfer.rx_buf = (unsigned long)rx_buffer;
	transfer.tx_buf = (unsigned long)tx_buffer;
	transfer.len = blocksize;

	fd = open("/dev/spidev1.0", O_RDONLY);
	if (fd < 0) {
		perror("could not open /dev/spidev1.0");
  }

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
    perror("SPI_IOC_WR_MAX_SPEED_HZ");
  }

	while ((blocknumber > 0) || (blocknumber == -1)) {
		for (offset = 0; offset < blocksize; offset += nb) {
			nb = read(STDIN_FILENO, & (tx_buffer[offset]), blocksize - offset);
			if (nb <= 0)
				break;
		}
		if (nb <= 0)
			break;

		if (ioctl(fd, SPI_IOC_MESSAGE(1), & transfer) < 0) {
			perror("SPI_IOC_MESSAGE");
			break;
		}
		if (write(STDOUT_FILENO, rx_buffer, blocksize) <= 0)
			break;
		if (blocknumber > 0)
			blocknumber--;
	}

	if (orig_speed != -1) {
		if ((res = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, & orig_speed)) < 0) {
			perror("SPI_IOC_WR_MAX_SPEED_HZ");
		}
	}

	if (blocknumber != 0)
		return -1;

	return res;
}

} // namespace linux_spi
