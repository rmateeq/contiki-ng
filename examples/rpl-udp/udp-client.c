//<<my includes>>
#include "cc2538-rf.h"
#include "dev/radio.h"
//#include "node-id.h"
//#include "os/net/mac/csma/csma-output.c"
//#include "net/mac/csma/csma.h"
//#include "net/packetbuf.h"

//#include "os/storage/cfs/cfs.h"
//>>my includes<<

#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"
#ifndef LOG_MODULE
#define LOG_MODULE "App"
#endif
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

/* macMinBE: Initial backoff exponent. Range 0--CSMA_MAX_BE */
 //#ifdef CSMA_CONF_MIN_BE
 //#define CSMA_MIN_BE CSMA_CONF_MIN_BE
 //#else
 //#define CSMA_MIN_BE 3
 //#endif
 
 /* macMaxBE: Maximum backoff exponent. Range 3--8 */
 //#ifdef CSMA_CONF_MAX_BE
 //#define CSMA_MAX_BE CSMA_CONF_MAX_BE
 //#else
 //#define CSMA_MAX_BE 5
 //#endif
 
/* macMaxCSMABackoffs: Maximum number of backoffs in case of channel busy/collision. Range 0--5 */
 //#ifdef CSMA_CONF_MAX_BACKOFF
 //#define CSMA_MAX_BACKOFF CSMA_CONF_MAX_BACKOFF
 //#else
 //#define CSMA_MAX_BACKOFF 5
 //#endif
 
 /* macMaxFrameRetries: Maximum number of re-transmissions attampts. Range 0--7 */
// #ifdef CSMA_CONF_MAX_FRAME_RETRIES
// #define CSMA_MAX_FRAME_RETRIES CSMA_CONF_MAX_FRAME_RETRIES
// #else
// #define CSMA_MAX_FRAME_RETRIES 7
// #endif

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
//<<set interval 10 instead of 60>>
#define SEND_INTERVAL     (2 * CLOCK_SECOND)
//>>set interval 10 instead of 60<<
//<<my vars>>
//int ps[4] = {39,74,109};
//staticchar msg[40] = "abcdefghijklmnopqrstuvwxyzabcdefghijklm";
// int iat[5] = {};
// int tp[4] = {};
// int mt[4] = {};
// #define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 16
// #define NETSTACK_CONF_RDC nullrdc_driver
// #define NETSTACK_CONF_MAC nullmac_driver
//>>my vars<<
static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
 const uip_ipaddr_t *sender_addr,
 uint16_t sender_port,
 const uip_ipaddr_t *receiver_addr,
 uint16_t receiver_port,
 const uint8_t *data,
 uint16_t datalen)
{
//  LOG_INFO("RSSI: %d",packetbuf_attr(PACKETBUF_ATTR_RSSI));
  //LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO("Received response from ");
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO("At node ");
  LOG_INFO_6ADDR(receiver_addr);
  int lqi_val;
  int rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi_val);
  LOG_INFO("lqi state %d",rd);
  LOG_INFO("lqi: %d",lqi_val);
  int rssi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi_val); //RADIO_PARAM_LAST_RSSI, RADIO_PARAM_LAST_PACKET_TIMESTAMP
  LOG_INFO("rssi state %d \n",rd);
  LOG_INFO("rssi: %d \n",rssi_val);

#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
  LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static int count = 1;
  static char str[120];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();


//<<set tx power>>
/*  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, CC2538_RF_TX_POWER_RECOMMENDED);
//  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, -16);
//update_txpower((int8_t)-16);
  printf("tp state: %d \n",rd); */
  int tp_val;
  int rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
 LOG_INFO("tp state %d \n",rd);
 LOG_INFO("tp: %d \n",tp_val); 
/*  int ch_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &ch_val);
  printf("channel state %d \n",rd);
  printf("current channel: %d \n",ch_val);
  int lqi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi_val);
  printf("lqi state %d \n",rd);
  printf("current lqi: %d \n",lqi_val);
  int rssi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi_val); //RADIO_PARAM_LAST_RSSI, RADIO_PARAM_LAST_PACKET_TIMESTAMP
  printf("rssi state %d \n",rd);
  printf("current rssi: %d \n",rssi_val);
//  int mt_val = 1;
//  rd = NETSTACK_RADIO.set_value(CSMA_MAX_FRAME_RETRIES, mt_val);
//  NETSTACK_RADIO.get_value(CSMA_MAX_FRAME_RETRIES, &mt_val);
//  printf("mt state %d \n",rd); */
 //Works end-to-end if UIP_CONF_TAG_TC_WITH_VARIABLE_RETRANSMISSIONS is set to 1.
 
 //printf("current mt: %d \n",PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS);
  //printf("current mt: %d \n",CSMA_MAX_FRAME_RETRIES); //1-UIP_MAX_MAC_TRANSMISSIONS_UNDEFINED (0),
 //2-UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS,4- CSMA_MAX_FRAME_RETRIES
/*  int minbe_val;
  rd = NETSTACK_RADIO.get_value(CSMA_MIN_BE, &minbe_val);
  printf("minbe state %d \n",rd);
  printf("current minbe: %d \n",minbe_val);
  int maxbe_val;
  rd = NETSTACK_RADIO.get_value(CSMA_MAX_BE, &maxbe_val);
  printf("maxbe state %d \n",rd);
  printf("current maxbe: %d \n",maxbe_val);
  printf("current buff: %d \n",PACKETBUF_SIZE); */
//  printf("tp after %d \n",RADIO_PARAM_TXPOWER);
//  int val;
//  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &val);
//  printf("....tp after %d \n",val);
//>>set tx power<<

/* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
    UDP_SERVER_PORT, udp_rx_callback);
  etimer_set(&periodic_timer, SEND_INTERVAL); //random_rand() % SEND_INTERVAL
  while(count <= 30) { //count <= 3 
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
     uipbuf_set_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS, 3);
 //packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS,3);
 //LOG_INFO("current mt: %d \n",uipbuf_get_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS));
    /* Send to DAG root */
      LOG_INFO("Sending request %d to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
     //abcdefghijklmnopqrstuvwxyzabcdefghij---36
     //abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef---110
      if (count < 10)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef00%d", count);
      else if (count < 100)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef0%d", count);
      else
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef%d", count);
   
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
  //  cfs_write (fp, str, sizeof(str));

      count++;
    } else {
      LOG_INFO("Not reachable yet\n");
    }

  /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }
  /* close the file*/  
// cfs_close (fp);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
