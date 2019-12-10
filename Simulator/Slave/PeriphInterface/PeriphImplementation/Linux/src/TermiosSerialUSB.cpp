#include <TermiosSerialUSB.hpp>
#include <cstring>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <Logger.hpp>

using namespace Linux;

TermiosSerialUSB::TermiosSerialUSB()
{
    m_fd = 0;
    std::memset(&m_tio, 0, sizeof(m_tio));
    m_modeBit = false;
    m_devicePath = nullptr;
}

TermiosSerialUSB::~TermiosSerialUSB()
{
}

bool TermiosSerialUSB::openComMDB(const char *devicePath, BaudRate baud)
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

    // Use space parity to get 3-byte sequence (0xff 0x00 <address>) on address byte
    m_tio.c_cflag |= CMSPAR;  // enable set "stick" parity (either mark or space)
    m_tio.c_cflag &= ~PARODD; // select space parity so that only address byte causes error

    // NOTE: The following block overrides PARMRK and PARENB bits cleared by cfmakeraw.
    m_tio.c_cflag |= PARENB;  // enable parity generation
    m_tio.c_iflag |= INPCK;   // enable parity checking
    m_tio.c_iflag |= PARMRK;  // enable in-band marking
    m_tio.c_lflag &= ~IGNPAR; // make sure parity errors are not ignored

    if (tcsetattr(m_fd, TCSANOW, &m_tio) == -1)
    {
        error << "COM: tcsetattr m_fd=" << m_fd << " on " << devicePath << "\n";
        return false;
    }

    m_devicePath = devicePath;

    return true;
}

bool TermiosSerialUSB::closeComMDB()
{
    return (close(m_fd) == 0);
}

int16_t TermiosSerialUSB::transmit(uint8_t *data, uint8_t dataLen)
{
    // mode bit on, address
    if (m_modeBit == true)
    {
        m_tio.c_cflag |= PARODD;
    }
    else
    {
        m_tio.c_cflag &= ~PARODD;
    }

    if (tcsetattr(m_fd, TCSADRAIN, &m_tio) == -1)
    {
        error << "COM: tcsetattr m_fd=" << m_fd << " on " << m_devicePath << "\n";
        return false;
    }

    tcflush(m_fd, TCIFLUSH);
    return write(m_fd, data, dataLen);
}

int16_t TermiosSerialUSB::receive(uint8_t *buffer, uint8_t bufferLen)
{
    return read(m_fd, buffer, bufferLen);
}

bool TermiosSerialUSB::hasData()
{
    uint32_t dataCount;
    ioctl(m_fd, FIONREAD, &dataCount);

    return dataCount > 0;
}

void TermiosSerialUSB::setModeBit(bool modeBit)
{
    m_modeBit = modeBit;
}

bool TermiosSerialUSB::getModeBit()
{
    return m_modeBit;
}
