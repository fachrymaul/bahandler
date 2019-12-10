#pragma once

// #include "MDBSerial.h"
// #include "Logger.h"
// #include <Arduino.h>
#include "../peripheral/inc/Serial.hpp"

#define RESET 					0x00
#define SETUP			 		0x01
#define POLL 					0x03
#define TYPE 					0x04
#define EXPANSION 				0x07
#define IDENTIFICATION 			0x00
#define FEATURE_ENABLE 			0x01

#define JUST_RESET 		 		0x0B

#define NO_RESPONSE 			2000

#define MAX_RESET 				5
#define MAX_RESET_POLL 			15

#define SETUP_TIME 				200
#define RESPONSE_TIME 			5

#define WARNING					1
#define ERROR					2
#define SEVERE					3

class MDBDevice
{
public:
	virtual bool Reset() = 0;

	virtual void Print() = 0;
	
protected:
	virtual int poll() = 0;
	
	IUart *m_mdb;

	int m_resetCount;
	
	int m_count;
	char m_buffer[64];

	char m_feature_level;
	unsigned int m_country;

	unsigned long m_manufacturer_code;
	uint8_t m_serial_number[12];
	uint8_t m_model_number[12];
};
