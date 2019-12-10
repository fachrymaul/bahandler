#ifndef MDB_H
#define MDB_H

// #include "MDBSerial.h"
// #include "Logger.h"
// #include <Arduino.h>
#include "../IMDBDevice.hpp"
#include "../../peripheral/inc/Serial.hpp"
#include "../../peripheral/inc/LinuxMillisTimer.hpp"

class BillAcceptor : MDBDevice
{
  public:
	BillAcceptor(Serial *mdb, Linux::MillisTimer *milis);
	void listen();
	bool Reset();
	void Print();
	void setup();
	bool init();
	int poll();
	void identification();
	void stacker();
	void billType(uint8_t *buff);
	void waitingAck();
	void escrow(uint8_t *buff);
	void security(uint8_t *buff);

  private:
	Serial *m_mdb;
	Linux::MillisTimer *m_milis;


	bool m_reset;
	uint16_t m_billEnable = 0;
	uint16_t m_escrowEnable = 0;
	uint16_t m_securityEnable = 0;
	uint8_t m_escrow = 0;


	const static uint8_t COUNTRY_CODE[2];
	uint8_t FEATURE_LEVEL = 0x01;
	const static uint8_t BILL_SCALING[2];
	uint8_t DECIMAL = 0x00;
	const static uint8_t STACKCER_CAPACITY[2];
	const static uint8_t BILL_TYPE[16];
	const static uint8_t SECURITY_LEVEL[2];

	uint8_t m_checksum;

	uint8_t ADDRESS = 0x30;
	uint8_t SECURITY = 0X02;
	uint8_t ESCROW = 0xFF;
	uint8_t STACKER = 0x06;
	uint8_t IDENTIFICATION_DATA = 0x00;
	uint8_t ACK = 0x00;
	const static uint8_t MANUFACTURER_CODE[4];
	const static uint8_t SW_VERSION[2];
	const static bool RESET_SUCCESS = true;

	const static uint8_t m_serial_number[13];
	const static uint8_t m_model_number[13];
	const static uint8_t JUST_RESET_TX[11];

	uint8_t m_stackerTx[2] = {0};
	uint16_t m_stackerCount = 0;
};

#endif // !MDB_H
