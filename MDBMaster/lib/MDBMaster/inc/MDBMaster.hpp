/**
 * @file MDBMaster.hpp
 * @brief 
 *
 * @copyright All Rights Reserved to Bukalapak
 * @author Bobby P. Johan
 * @author Richard
 */
#ifndef MDB_MASTER_H
#define MDB_MASTER_H

#include <../../Serial/inc/SerialEsp.hpp>
#include <stdint.h>
#include "Master2Proxy.hpp"

class MDBMaster
{
public:

    struct MdbInfo
    {
        uint64_t manfCode;
        uint16_t countryCode;
        uint16_t stackedBill;
        uint8_t  activeBillType;
        uint8_t  statusCode;
    };

    MDBMaster();
    /**
     * Function acceptMoney
     * This function handle money insert to bill stack and return value insert
     * @return BillType 
     * example 0 = 1k
     *         1 = 2k
     *         2 = 5k
     *         3 = 10k
     *         4 = 20k
     *         5 = 50k 
     *         6 = 100k
     */
    uint8_t acceptMoney();
    /**
     * Function setup 
     * This function handle Bill Acceptor setup and setup 
     * @param *serialProxy Serial Interface for proxy  
     * @return true for success cmd
     * @return false for failed cmd
     */
    bool setup(ISerial *serialProxy);

    /**
     * Function Reset
     * This function for Reset Bill Acceptor
     * @return true for success cmd
     * @return false for failed cmd
     */
    bool reset();

    /**
     * Function getInfo
     * @return MdbInfo : manufacturer code, country code , active bill type, stacked bill, status Code 
     */
    MdbInfo getInfo();

    /**
     * Enable Bill Type want to insert to bill acceptor
     * @param billType to enable money 1k - 100k
     * @return true for success cmd
     * @return false for failed cmd
     */
    bool enableBillType(M2P::BillTypeSwitch billType);

    /**
     * Disable Bill Type want to insert to bill acceptor
     * @param billType to enable money 1k - 100k
     * @return true for success cmd
     * @return false for failed cmd
     */
    bool disBillType(M2P::BillTypeSwitch billType);

    /**
     * Reset Bill Type want to insert to bill acceptor 
     * @return true for success cmd
     * @return false for failed cmd
     */
    bool resetBillType();

    /**
     * Reset Money In internal variable 
     * @return true for success cmd
     * @return false for failed cmd
     */
    bool resetMoney();

private:
    bool sendData(const uint8_t *data, uint32_t length);
    ISerial  *m_serialProxy;
    MdbInfo  m_mdbInfo;
    uint8_t  m_billType;
    uint32_t m_money;
};

#endif
