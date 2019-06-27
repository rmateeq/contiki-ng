//<<my includes>>
#include "cc1200-const.h"
#include "cc1200-conf.h"
#include "cc1200-arch.h"
#include "cc1200-rf-cfg.h"
#include "dev/radio.h"
#include "node-id.h"
#include "os/net/mac/csma/csma-output.c"
//#include "os/net/mac/csma/csma.h"

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
#ifndef LOG_LEVE:
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
//<<set interval 10 instead of 60>>
#define SEND_INTERVAL     (1 * CLOCK_SECOND)
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
  LOG_INFO("RSSI: %f",packetbuf_get_attr(PACKETBUF_ATTR_RSSI));
  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static int count = 1;
  static char str[40];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();


//<<set tx power>>
//printf("tp before %d \n",RADIO_PARAM_TXPOWER);
//  int val1;
//  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &val1);
 // printf("....tp before %d \n",val1);
//radio_value_t tx_level = -16;
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, CC1200_CONST_TX_POWER_MIN);
//  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, -16);
//update_txpower((int8_t)-16);
  printf("tp state: %d \n",rd);
  int ch_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &ch_val);
  printf("channel state %d \n",rd);
  printf("current channel: %d \n",ch_val);
  int lqi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi_val);
  printf("lqi state %d \n",rd);
  printf("current lqi: %d \n",lqi_val);
  int rssi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_RSSI, &rssi_val);
  printf("rssi state %d \n",rd);
  printf("current rssi: %d \n",rssi_val);
  int mt_val;
  rd = NETSTACK_RADIO.get_value(CSMA_MAX_FRAME_RETRIES, &mt_val);
  printf("mt state %d \n",rd);
  printf("current mt: %d \n",mt_val);
  int minbe_val;
  rd = NETSTACK_RADIO.get_value(CSMA_MIN_BE, &minbe_val);
  printf("minbe state %d \n",rd);
  printf("current minbe: %d \n",minbe_val);
  int maxbe_val;
  rd = NETSTACK_RADIO.get_value(CSMA_MAX_BE, &maxbe_val);
  printf("maxbe state %d \n",rd);
  printf("current maxbe: %d \n",maxbe_val);
  printf("current buff: %d \n",PACKETBUF_SIZE);
//  printf("tp after %d \n",RADIO_PARAM_TXPOWER);
//  int val;
//  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &val);
//  printf("....tp after %d \n",val);
//>>set tx power<<

/* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
    UDP_SERVER_PORT, udp_rx_callback);
  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(count <= 3 ) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
    /* Send to DAG root */
      LOG_INFO("Sending request %d to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      if (count < 10)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghij00%d", count);
      else if (count < 100)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghij0%d", count);
      else
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghij%d", count);
   
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
