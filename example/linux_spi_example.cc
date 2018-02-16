#include "../include/linux_spi/linux_spi.hpp"

#include <cstdint>
#include <stdio.h>
#include <string.h>

int main(void) {
  using namespace linux_spi;
  
  spi_config config{3, 8, 16*1000*1000};
  spi_state state{"/dev/spidev1.0", std::move(config)};
  register_errno_logging(printf);

  OpenFailure err = spi_open(state);
  if (err != OpenFailure::NONE)
    printf("spi opened with failure %d", err);

  const std::size_t buf_size = 48;
  std::uint8_t tx_buffer[buf_size];
  std::uint8_t rx_buffer[buf_size];
  memset(tx_buffer, 0, buf_size);
  memset(rx_buffer, 0, buf_size);
  tx_buffer[0] = 0x01;
  tx_buffer[1] = 0x36;
  
  printf("sending %s, to spidev2.0 in full duplex \n ", tx_buffer);
  spi_xfer(state.fd(), tx_buffer, buf_size, rx_buffer, buf_size);

  printf("rx_buffer=%s\n", rx_buffer);
  spi_close(state.fd()); 
  
  return 0;
}

