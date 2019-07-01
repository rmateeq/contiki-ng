#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

//#define NETSTACK_CONF_RDC nullrdc_driver //
#define NETSTACK_CONF_MAC nullmac_driver //tschmac_driver,csma_driver,sensniff_mac_driver
//#define NETSTACK_CONF_FRAMER  framer_802154
//#define NETSTACK_CONF_RADIO   cc2538_rf_driver

//#define RS232_BAUDRATE USART_BAUD_38400

// Re-Mote channel and power configuration
#ifndef CC2538_RF_CONF_CHANNEL
#define CC2538_RF_CONF_CHANNEL 26
#endif /* CC2538_RF_CONF_CHANNEL */
//#define CC2538_RF_CONF_TX_POWER -40

// CSMA MAC Parameters
#define CSMA_CONF_MAX_BE 5
#define CSMA_CONF_MIN_BE 0
#define CSMA_CONF_MAX_FRAME_RETRIES 7

// TSCH MAC Parameters
#define TSCH_CONF_MAC_MAX_BE 5
#define TSCH_CONF_MAC_MAX_BE 1
#define TSCH_CONF_MAC_MAX_FRAME_RETRIES 7
#endif /* PROJECT_CONF_H_ */
