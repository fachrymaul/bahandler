#include <TermiosSerialDummy.hpp>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <Logger.hpp>

using namespace Linux;

TermiosSerialDummy::TermiosSerialDummy()
{
    m_fd = 0;
    std::memset(&m_tio, 0x00, sizeof(m_tio));
    m_modeBit = false;
    m_devicePath = nullptr;
    clearQueue();
}

TermiosSerialDummy::~TermiosSerialDummy()
{
}

bool TermiosSerialDummy::openComMDB(const char *devicePath, BaudRate baud)
{
    m_fd = open(devicePath, O_RDWR | O_NONBLOCK | O_NOCTTY);

    if (m_fd < 0)
    {
        error << "COM: Could not open " << devicePath << "\n";
        return false;
    }

    tcgetattr(m_fd, &m_tio);
    cfmakeraw(&m_tio);
    cfsetspeed(&m_tio, baud);

    // Ignore modem lines and enable receiver
    m_tio.c_cflag |= (CLOCAL | CREAD);

    // No flow control
    m_tio.c_cflag &= ~CRTSCTS;        // No HW flow control
    m_tio.c_iflag &= ~(IXON | IXOFF); // Set the input flags to disable in-band flow control

    // Set bits per byte
    m_tio.c_cflag &= ~CSIZE;
    m_tio.c_cflag |= CS8;

    // Timing and char read
    m_tio.c_cc[VTIME] = 0;
    m_tio.c_cc[VMIN] = 0;

    // Flush serial buffer and set new conf
    tcflush(m_fd, TCIFLUSH);
    if (tcsetattr(m_fd, TCSANOW, &m_tio) == -1)
    {
        error << "COM: tcsetattr m_fd=" << m_fd << " on " << devicePath << "\n";
        return false;
    }

    m_devicePath = devicePath;

    return true;
}

bool TermiosSerialDummy::closeComMDB()
{
    return (close(m_fd) == 0);
}

int16_t TermiosSerialDummy::transmit(uint8_t *data, uint8_t dataLen)
{
    uint8_t hdrLen = 0;
    uint8_t totalLen = 0;

    // mode bit on, address
    if (m_modeBit == true)
    {
        hdrLen = sizeof((uint16_t)TERM_DUMMY_MODEBIT_HDR) / sizeof(uint8_t);
        uint8_t hdr[hdrLen];
        hdr[0] = (TERM_DUMMY_MODEBIT_HDR >> 8) & 0xFF;
        hdr[1] = (TERM_DUMMY_MODEBIT_HDR & 0xFF);
        totalLen = write(m_fd, hdr, hdrLen);
        if (totalLen == -1)
        {
            return totalLen;
        }
    }

    // tcflush(m_fd, TCIFLUSH);
    totalLen += write(m_fd, data, dataLen);

    return totalLen;
}

int16_t TermiosSerialDummy::receive(uint8_t *buffer, uint8_t bufferLen)
{
    return read(m_fd, buffer, bufferLen);
}

bool TermiosSerialDummy::queue(uint8_t *data, uint8_t dataLen)
{
    // buffer overflow
    if (idxQueue + dataLen > ISerial::TX_QUEUE_BUFF_MAX)
    {
        return false;
    }

    // mode bit on, add header
    uint8_t hdrLen = 0;
    if (m_modeBit == true)
    {
        hdrLen = sizeof((uint16_t)TERM_DUMMY_MODEBIT_HDR) / sizeof(uint8_t);
        uint8_t hdr[hdrLen];
        hdr[0] = (TERM_DUMMY_MODEBIT_HDR >> 8) & 0xFF;
        hdr[1] = (TERM_DUMMY_MODEBIT_HDR & 0xFF);
        std::memcpy((uint8_t *)&m_txQueue[idxQueue], hdr, dataLen);
        idxQueue += hdrLen;
    }

    std::memcpy((uint8_t *)&m_txQueue[idxQueue], data, dataLen);
    idxQueue += dataLen;

    return true;
}

int16_t TermiosSerialDummy::transmitQueue()
{
    if (idxQueue > 0)
    {
        int16_t stat;
        m_modeBit = false;
        stat = transmit(m_txQueue, idxQueue);
        clearQueue();
        return stat;
    }
    return 0;
}

void TermiosSerialDummy::clearQueue()
{
    idxQueue = 0;
    std::memset(m_txQueue, 0x00, sizeof(m_txQueue));
}

bool TermiosSerialDummy::hasData()
{
    uint32_t dataCount;
    ioctl(m_fd, FIONREAD, &dataCount);

    return dataCount > 0;
}

void TermiosSerialDummy::setModeBit(bool modeBit)
{
    m_modeBit = modeBit;
}

bool TermiosSerialDummy::getModeBit()
{
    return m_modeBit;
}
