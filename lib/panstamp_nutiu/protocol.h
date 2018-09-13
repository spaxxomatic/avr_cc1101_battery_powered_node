
#define MAX_RETRY_SEND_DATA 10
#define MAX_RETRY_SEND_ACK 10

#define ACK_PACKET 0XF0F0
#define CMD_SET_RADIO_ADDRESS 0xDD
#define CMD_SET_ACTOR 0x01
#define CMD_READ_INPUT 0x02

enum CUSTOM_REGINDEX                    
{                                       
  REGI_PRODUCTCODE = 0,                 
  REGI_HWVERSION,
  REGI_FWVERSION,
  REGI_SYSSTATE, 
  REGI_FREQCHANNEL,                     
  REGI_NETWORKID,                       
  REGI_DEVADDRESS,                      
  REGI_TXINTERVAL,
}

