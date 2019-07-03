#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/*** Application Layer ***/
// traffic rate (inter-packet delay in seconds) [4 * 3 options]
//#define SEND_INTERVAL (1 * CLOCK_SECOND)

//Current App layer Packet Size out of [45,80, and 115]
//#define PKT_SIZE 45

//Total number of packets to be transmitted against each parameter configuration
//#define NUM_PKTS 200

//Enforce MAC_MAX_TRANSMISSIONS across all nodes
//#define UIP_CONF_TAG_TC_WITH_VARIABLE_RETRANSMISSIONS 1
/*** Transport Layer ***/
// All communication is carried out using UDP

/*** Network Layer ***/
// RPL Parameters [2 options]
// #define RPL_CONF_SUPPORTED_OFS
// #define RPL_CONF_OF_OCP
// #define RPL_CONF_PROBING_SEND_FUNC
// #define LINK_STATS_CONF_INIT_ETX_FROM_RSSI
// #define RPL_MRHOF_CONF_SQUARED_ETX

/*** MAC Layer ***/
// select MAC protocol [2+1 options]
//#define NETSTACK_CONF_MAC nullmac_driver //tschmac_driver,csma_driver

// CSMA MAC Parameters [1-4-3 options]
//#ifndef CSMA_CONF_MAX_BE
//#define CSMA_CONF_MAX_BE 5
//#endif
//#ifndef CSMA_CONF_MIN_BE
//#define CSMA_CONF_MIN_BE 0
//#endif
//#ifndef CSMA_CONF_MAX_FRAME_RETRIES
//#define CSMA_CONF_MAX_FRAME_RETRIES 7
//#endif

// TSCH MAC Parameters [1-4-3 options] 
//#ifndef TSCH_CONF_MAC_MAX_BE
//#define TSCH_CONF_MAC_MAX_BE 5
//#endif
//#ifndef TSCH_CONF_MAC_MIN_BE
//#define TSCH_CONF_MAC_MAX_BE 1
//#endif
//#ifndef TSCH_CONF_MAC_MAX_FRAME_RETRIES
//#define TSCH_CONF_MAC_MAX_FRAME_RETRIES 7
//#endif
//#define NETSTACK_CONF_RADIO   cc2538_rf_driver
//#define RS232_BAUDRATE USART_BAUD_38400

/*** PHY Layer ***/
// Re-Mote channel and power configuration 
//#ifndef CC2538_RF_CONF_CHANNEL
//#define CC2538_RF_CONF_CHANNEL 26
//#endif /* CC2538_RF_CONF_CHANNEL */
// Transmissions power [4 options] 0x00(-24),42(-15),58(-13),62(-11),72(-9),88(-7),91(-5),A1(-3),B0(-1),B6(0),C5(1),D5(3),ED(5),FF(7)
//#ifndef CC2538_RF_CONF_TX_POWER
#define CC2538_RF_CONF_TX_POWER 0x00
//#endif
/***Logging***/
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_INFO
//#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_DBG
//#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
//#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
//#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG

#endif /* PROJECT_CONF_H_ */
