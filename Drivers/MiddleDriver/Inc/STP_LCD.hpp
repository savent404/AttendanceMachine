#pragma once

#include "usart.h"

class STP_LCD {
public:
  STP_LCD(UART_HandleTypeDef *uart) {
    handle = uart;
  }
  ~STP_LCD() {

  }
private:

  void Send(uint8_t *buffer, size_t cnt) {
    
  }
  UART_HandleTypeDef *handle;
};