#ifndef MASTER_TO_PROXY_HPP
#define MASTER_TO_PROXY_HPP

namespace M2P
{
enum BillTypeSwitch
{
    SW_BILL_NONE = 0x00,
    SW_BILL_1K   = 0x01,
    SW_BILL_2K   = 0x02,
    SW_BILL_5K   = 0x04,
    SW_BILL_10K  = 0x08,
    SW_BILL_20K  = 0x10,
    SW_BILL_50K  = 0x20,
    SW_BILL_100K = 0x40,
    SW_BILL_ALL  = 0xFF,
};

enum BukaBvMdbCmd
{
    CMD_RESET                       = 0x00,
    CMD_SETUP                       = 0x01,
    CMD_SETUP_BV_FEATURE            = 0x02,
    CMD_SETUP_COUNTRY_CURRENCY_CODE = 0x03,
    CMD_SETUP_BILL_SCALING_FACTOR   = 0x04,
    CMD_SETUP_DECIMAL_PLACE         = 0x05,
    CMD_SETUP_STACKER_CAP           = 0x06,
    CMD_SETUP_BILL_SECURITY_LVL     = 0x07,
    CMD_SETUP_ESCROW_NOESCROW       = 0x08,
    CMD_SETUP_BILL_TYPE             = 0x09,

    CMD_SECURITY_LOW  = 0x11,
    CMD_SECURITY_HIGH = 0x12,
    
    CMD_POLL = 0x21,

    CMD_BILL_ENBL_ALL  = 0x30,
    CMD_BILL_ENBL_1K   = 0x31,
    CMD_BILL_ENBL_2K   = 0x32,
    CMD_BILL_ENBL_5K   = 0x33,
    CMD_BILL_ENBL_10K  = 0x34,
    CMD_BILL_ENBL_20K  = 0x35,
    CMD_BILL_ENBL_50K  = 0x36,
    CMD_BILL_ENBL_100K = 0x37,
    CMD_BILL_DSBL_ALL  = 0x38,
    CMD_BILL_DSBL_1K   = 0x39,
    CMD_BILL_DSBL_2K   = 0x3A,
    CMD_BILL_DSBL_5K   = 0x3B,
    CMD_BILL_DSBL_10K  = 0x3C,
    CMD_BILL_DSBL_20K  = 0x3D,
    CMD_BILL_DSBL_50K  = 0x3E,
    CMD_BILL_DSBL_100K = 0x3F,

    CMD_ESCROW_IN    = 0x41,
    CMD_ESCROW_OUT   = 0x42,
    CMD_BILL_STACKED = 0x51,
    CMD_EXPANSION    = 0x61,
    CMD_ACCEPT_MONEY = 0x71,
    CMD_GET_INFO     = 0x81,
    CMD_ACK          = 0x0F,
    CMD_NACK         = 0xFF,
    CMD_RET          = 0xAA,
    CMD_POLL_ACK     = 0xFE,
};
} // namespace M2P

#endif
