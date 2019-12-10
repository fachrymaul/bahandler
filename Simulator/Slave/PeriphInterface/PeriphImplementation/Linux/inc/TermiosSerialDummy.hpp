#ifndef TERMIOS_SERIAL_DUMMY_H
#define TERMIOS_SERIAL_DUMMY_H

#include <IPeriphInterface.hpp>
#include <termios.h>
#include <cstdint>

namespace Linux
{
class TermiosSerialDummy final : public ISerial
{
  private:
    const uint16_t TERM_DUMMY_MODEBIT_HDR = 0xFF00;

  public:
    enum BaudRate
    {
        BAUD_50 = B50,
        BAUD_75 = B75,
        BAUD_110 = B110,
        BAUD_134 = B134,
        BAUD_150 = B150,
        BAUD_200 = B200,
        BAUD_300 = B300,
        BAUD_600 = B600,
        BAUD_1200 = B1200,
        BAUD_1800 = B1800,
        BAUD_2400 = B2400,
        BAUD_4800 = B4800,
        BAUD_9600 = B9600,
        BAUD_19200 = B19200,
        BAUD_38400 = B38400,
        BAUD_57600 = B57600,
        BAUD_115200 = B115200,
        BAUD_230400 = B230400
    };

    TermiosSerialDummy();
    ~TermiosSerialDummy();

    bool openComMDB(const char *devicePath, BaudRate baud);
    bool closeComMDB();
    int16_t transmit(uint8_t *data, uint8_t dataLen);
    int16_t receive(uint8_t *buffer, uint8_t bufferLen);
    bool queue(uint8_t *data, uint8_t dataLen);
    int16_t transmitQueue();
    void clearQueue();
    bool hasData();

    void setModeBit(bool modeBit);
    bool getModeBit();

  private:
    int m_fd;
    bool m_modeBit;
    const char *m_devicePath;
    struct termios m_tio;
};
} // namespace Linux

#endif
