#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
/*** Sensniff ***/
//#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 { addr_contexts[0].prefix[0]=0xbb; addr_contexts[0].prefix[1]=0xbb; }
/*** Application Layer ***/
// traffic rate (inter-packet delay in seconds) [4 * 3 options]
//#define SEND_INTERVAL (1 * CLOCK_SECOND)

//Current App layer Packet Size out of [45,80, and 115]
//#define PKT_SIZE 45
//#define PACKETBUF_CONF_SIZE in bytes
//RADIO_CONST_MAX_PAYLOAD_LEN, MAX_PAYLOAD_LEN, TSCH_PACKET_MAX_LEN
//MAX_NUM_QUEUED_PACKETS,
//Total number of packets to be transmitted against each parameter configuration
//#define NUM_PKTS 200
#define UIP_CONF_UDP_CONNS 25
//Enforce MAC_MAX_TRANSMISSIONS across all nodes
#define UIP_CONF_TAG_TC_WITH_VARIABLE_RETRANSMISSIONS 1
/*** Transport Layer ***/
// All communication is carried out using UDP

/*** Network Layer ***/
// Parameters [2 options]
//#define UIP_CONF_IPV6 1
//#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 10 //default 16
//#endif

//#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE 165
//#endif
//#define UIP_CONF_IPV6_QUEUE_PKT 1

//#ifndef UIP_CONF_ROUTER
//#define UIP_CONF_ROUTER 1
//#endif

//#define UIP_CONF_IPV6_RPL 1
//-----------------------------------------------------------------------------
//#define UIP_CONF_ND6_SEND_RA 1
//#define UIP_CONF_IP_FORWARD  1
 #define RPL_CONF_SUPPORTED_OFS {&rpl_of0, &rpl_mrhof}
 #define RPL_CONF_OF_OCP RPL_OCP_MRHOF //_OF0
 #define RPL_MRHOF_CONF_SQUARED_ETX 1
 #define RPL_CONF_TRICKLE_REFRESH_DAO_ROUTES 6 //by default dao refreshal is done every 4secs.
 #define RPL_CONF_DIO_INTERVAL_MIN 12 //13 will result in 8sec. default is 12, resulting in 4 sec.
 #define RPL_CONF_DEFAULT_LIFETIME_UNIT 60 //defualt redefined due to confusion in rpl-conf.h
 #define RPL_CONF_DEFAULT_LIFETIME 30 // same as above
 #define RPL_CONF_DAO_RETRANSMISSION_TIMEOUT (6 * CLOCK_SECOND) //default 5
// #define RPL_CONF_PROBING_SEND_FUNC
// #define LINK_STATS_CONF_INIT_ETX_FROM_RSSI
// #define RPL_MRHOF_CONF_SQUARED_ETX

/*** MAC Layer ***/
// select MAC protocol [2+1 options]
#define NETSTACK_CONF_MAC tschmac_driver //csma_driver //nullmac_driver,
//#define TSCH_CONF_AUTOSTART 0
#define LLSEC802154_CONF_ENABLED 0
#define SICSLOWPAN_CONF_FRAG 0
// CSMA MAC Parameters [1-4-3 options]
#define QUEUEBUF_CONF_NUM 8 //default 8
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
//#define CC2538_RF_CONF_TX_POWER 0xC5
#define ENERGEST_CONF_ON 1
//#endif
/***Logging***/
//#define LOG_CONF_LEVEL_IPV6                        LOG_LEVEL_INFO
#define LOG_CONF_LEVEL_RPL                         LOG_LEVEL_DBG
//#define LOG_CONF_LEVEL_6LOWPAN                     LOG_LEVEL_DBG
//#define LOG_CONF_LEVEL_TCPIP                       LOG_LEVEL_DBG
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
//#define LOG_CONF_LEVEL_FRAMER                      LOG_LEVEL_DBG
//#define LOG_LEVEL_NULLNET                          LOG_LEVEL_DBG
//#define LOG_LEVEL_6TOP                             LOG_LEVEL_DBG
//#define LOG_LEVEL_LWM2M                            LOG_LEVEL_DBG
//#define LOG_LEVEL_MAIN                             LOG_LEVEL_DBG
#endif /* PROJECT_CONF_H_ */
