#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_
//Enforce MAC_MAX_TRANSMISSIONS across all nodes
//#define UIP_CONF_TAG_TC_WITH_VARIABLE_RETRANSMISSIONS 1

//#ifndef UIP_CONF_BUFFER_SIZE
//#define UIP_CONF_BUFFER_SIZE 165
//#endif
//#define UIP_CONF_ROUTER 0
/*** MAC Layer ***/
// select MAC protocol [2+1 options]
#define NETSTACK_CONF_MAC tschmac_driver //csma_driver //nullmac_driver,
//#define TSCH_CONF_AUTOSTART 0
#define LLSEC802154_CONF_ENABLED 0
#define SICSLOWPAN_CONF_FRAG 0
#define ENERGEST_CONF_ON 1
//#endif
/***Logging***/
#define LOG_CONF_LEVEL_MAC                         LOG_LEVEL_DBG
#endif /* PROJECT_CONF_H_ */
