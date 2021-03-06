
// TODO: integrate into the build.

enum BaudRate
{
  BAUD_115200 = 1,
  BAUD_57600 = 2,
  BAUD_38400 = 3,
  BAUD_28800 = 4,
  BAUD_23040 = 5,
  BAUD_19200 = 6,
  BAUD_14400 = 8,
  BAUD_12800 = 9,
  BAUD_11520 = 10,
  BAUD_9600 = 12,
  BAUD_7680 = 15
};

void serial0_init(int divisor);
void serial0_write(char *buffer, size_t length);