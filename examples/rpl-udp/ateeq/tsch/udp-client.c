//<<my includes>>
#include "cc2538-rf.h"
#include "dev/radio.h"
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

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
//<<set interval 10 instead of 60>>
#define SEND_INTERVAL     (4 * CLOCK_SECOND)
//>>set interval 10 instead of 60<<
//<<my vars>>
//>>my vars<<
static struct simple_udp_connection udp_conn;
unsigned long ct_start;
unsigned long ct_end;
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


/* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
    UDP_SERVER_PORT, udp_rx_callback);
   etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL); //
     ct_start = clock_seconds();
   printf("start time: %lu\n", ct_start);
for (i = 0; i <= 5; i++ ){
 // Transmissions power [4 options] 0x00(-24),42(-15),58(-13),62(-11),72(-9),88(-7),91(-5),A1(-3),B0(-1),B6(0),C5(1),D5(3),ED(5),FF(7)
//#ifndef CC2538_RF_CONF_TX_POWER
//#define CC2538_RF_CONF_TX_POWER 0xC5

 int tp_val;
 if (i == 0){
  tp_val = -3
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  LOG_INFO("tp state %d",rd);
  LOG_INFO("new tp %d",tp_val);
 }
 else if (i == 1){
  tp_val = -1
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  LOG_INFO("tp state %d",rd);
  LOG_INFO("new tp %d",tp_val);
 }
 else if (i == 2){
  tp_val = 0
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  LOG_INFO("tp state %d",rd);
  LOG_INFO("new tp %d",tp_val);
 }
 else if (i == 3){
  tp_val = 1
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  LOG_INFO("tp state %d",rd);
  LOG_INFO("new tp %d",tp_val);
 }
 else if (i == 4){
  tp_val = 3
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  LOG_INFO("tp state %d",rd);
  LOG_INFO("new tp %d",tp_val);
 }
 else if (i == 5){
  tp_val = 5
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  LOG_INFO("tp state %d",rd);
  LOG_INFO("new tp %d",tp_val);
 }
 LOG_INFO_("...............................................................................")'
 LOG_INFO_("..............................NEW RUN..........................................");
 LOG_INFO_("...............................................................................");
  while(count <= 300) { //count <= 3    
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
     printf("reachability time: %lu\n", clock_seconds());
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
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs00%d", count);
      else if (count < 100)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs0%d", count);
      else
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs%d", count);
   
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
  //  cfs_write (fp, str, sizeof(str));

      count++;
     if ((clock_seconds() - ct_start) > 3000) {
      printf("clock difference abrupt: %lu\n", (clock_seconds() - ct_start));
      break;
     }
     else if (count == 301) {
       ct_end = clock_seconds();
      printf("clock difference: %lu\n", (ct_end - ct_start));
     }
    } else {
      LOG_INFO("Not reachable yet\n");
    }

  /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  } //while ends here
}//for ends here
  /* close the file*/  
// cfs_close (fp);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
