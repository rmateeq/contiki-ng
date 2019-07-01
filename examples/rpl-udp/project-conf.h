#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define NETSTACK_CONF_RDC nullrdc_driver
#define NETSTACK_CONF_MAC nullmac_driver
#define NETSTACK_CONF_FRAMER  framer_802154
#define NETSTACK_CONF_RADIO   rf230_driver

#define RS232_BAUDRATE USART_BAUD_38400

#define CSMA_CONF_MAX_BE 5
#define CSMA_CONF_MIN_BE 0
#define CSMA_CONF_MAX_FRAME_RETRIES 7

#endif /* PROJECT_CONF_H_ */
