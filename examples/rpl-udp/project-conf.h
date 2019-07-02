#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

/*** Application Layer ***/
// traffic rate (inter-packet delay in seconds) [4 * 3 options]
#define APP_CONF_SEND_INTERVAL (1 * CLOCK_SECOND)
// application packet size in bytes [3 options]
#define APP_CONF_PKT_SIZE 40

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
#define NETSTACK_CONF_MAC nullmac_driver //tschmac_driver,csma_driver

// CSMA MAC Parameters [1-4-3 options]
#define CSMA_CONF_MAX_BE 5
#define CSMA_CONF_MIN_BE 0
#define CSMA_CONF_MAX_FRAME_RETRIES 7

// TSCH MAC Parameters [1-4-3 options] 
#define TSCH_CONF_MAC_MAX_BE 5
#define TSCH_CONF_MAC_MAX_BE 1
#define TSCH_CONF_MAC_MAX_FRAME_RETRIES 7

//#define NETSTACK_CONF_RADIO   cc2538_rf_driver
//#define RS232_BAUDRATE USART_BAUD_38400

/*** PHY Layer ***/
// Re-Mote channel and power configuration 
#ifndef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL 26
#endif /* CC2538_RF_CONF_CHANNEL */
// Transmissions power [4 options]
//#define CC2538_RF_CONF_TX_POWER -40

/***Logging***/
#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG

#endif /* PROJECT_CONF_H_ */
