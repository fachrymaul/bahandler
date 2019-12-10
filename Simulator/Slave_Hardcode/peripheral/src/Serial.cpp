#include "../inc/Serial.hpp"
#include <termios.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

Serial::CmdStatus Serial::transmit(uint8_t *txData, uint32_t txDatalen)
{
    if (write(m_serialPort, txData, txDatalen) == -1)
    {
        return CMD_UART_ERROR;
    }

    return CMD_SUCCESS;
}

Serial::CmdStatus Serial::receive(uint8_t *rxBuffer, uint32_t rxBufferSize, uint32_t *pRxDataLen)
{
    int32_t rxLen = 0;
    rxLen = read(m_serialPort, rxBuffer, rxBufferSize);

    if (rxLen == -1)
    {
        return CMD_UART_ERROR;
    }

    *pRxDataLen = rxLen;
    return CMD_SUCCESS;
}

int Serial::initialize(const char *PORT)
{
    struct termios serialConf;
    memset(&serialConf, 0, sizeof(serialConf));

    m_serialPort = open(PORT, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_serialPort < -1)
    {
        std::cout << "failed to open part" << std::endl;
        return -1;
    }
    
    tcgetattr(m_serialPort, &serialConf);
    cfmakeraw(&serialConf);
    cfsetspeed(&serialConf, B9600);
    
    // Ignore modem lines and enable receiver
    serialConf.c_cflag |= (CLOCAL | CREAD);
    
    // No flow control
    serialConf.c_cflag &= ~CRTSCTS;        // No HW flow control
    serialConf.c_iflag &= ~(IXON | IXOFF); // Set the input flags to disable in-band flow control
    
    // Set bits per byte
    serialConf.c_cflag &= ~CSIZE;
    serialConf.c_cflag |= CS8;
    
    // Use space parity to get 3-byte sequence (0xff 0x00 <address>) on address byte
    serialConf.c_cflag &= ~CMSPAR; // disable set "stick" parity (either mark or space)
    serialConf.c_cflag &= ~PARODD; // select space parity so that only address byte causes error
    
    // NOTE: The following block overrides PARMRK and PARENB bits cleared by cfmakeraw.
    serialConf.c_cflag &= ~PARENB; // disable parity generation
    serialConf.c_iflag &= ~INPCK;  // disable parity checking
    serialConf.c_iflag &= ~PARMRK; // disable in-band marking
    serialConf.c_lflag |= IGNPAR;  // make sure parity errors are ignored
    
    tcflush(m_serialPort, TCIFLUSH);
    tcsetattr(m_serialPort, TCSANOW, &serialConf);

    return 0;
}

void Serial::closePort()
{
    close(m_serialPort);
}

uint32_t Serial::getDataCount()
{
    return 0;
}

uint16_t Serial::getErrorCode()
{
    return 0;
}

Serial::CmdStatus Serial::transmitAsync(uint8_t *txData, uint32_t txDataLen, TxCallbackConf *pFinishedCbConf)
{
    return CMD_SUCCESS;
}

Serial::CmdStatus Serial::startReceiveAsync(RxCallbackConf *pReceivedCbConf)
{
    return CMD_SUCCESS;
}

Serial::CmdStatus Serial::stopReceiveAsync()
{
    return CMD_SUCCESS;
}