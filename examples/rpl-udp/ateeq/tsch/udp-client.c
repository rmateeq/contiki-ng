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
//#define SEND_INTERVAL     (4 * CLOCK_SECOND)
//>>set interval 10 instead of 60<<
//<<my vars>>
unsigned long ct_start;
//unsigned long ct_end;
static int tp[4] = {-11,-5,1,7}; //[-13,-9,-5,-1,1,3,5];
static int ps[2] = {25,100}; //[25,50,75,100];
static int mt[2] = {1,5}; //[1,2,3,4,5];
//bidirectional:yes,no
static int iat[3] = {2,6,10}; //[1,2,4,6,8,10];
static int tp_c = 0;
  static int ps_c = 0;
  static int mt_c = 0;
  static int iat_c = 0;
static int mts = 0;
static int SEND_INTERVAL = 0;

  static int i = 0;
//number of nodes: 8(d,s),16(d,s),24,32
//dt: real
//mac: tsch,mac
//of: 0,mrh
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
  
   
  for (; tp_c <= 3; tp_c++ ){
    ct_start = clock_seconds();
  printf("%d start time:::: %lu\n",i, ct_start);

   // Transmissions power [4 options] 0x00(-24),42(-15),58(-13),62(-11),72(-9),88(-7),91(-5),A1(-3),B0(-1),B6(0),C5(1),D5(3),ED(5),FF(7)
   int tp_val = -99;
   int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp[tp_c]);
   rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
   printf("tp state:::: %d",rd);
   printf("new tp:::: %d",tp_val);
  
  for (; ps_c <= 1; ps_c++ ){
   //static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
   //for (int j = 0; j < ps[ps_c] - 3; ++j) {
   //     str[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
   // }
   // str[ps[ps_c]-3] = 0;
    printf("new ps:::: %d",ps[ps_c]);
   
   for (; iat_c <= 2; iat_c++ ){
    SEND_INTERVAL = (iat[iat_c] * CLOCK_SECOND);
    printf("new iat:::: %d",iat[iat_c]);
   
   for (; mt_c <= 1; mt_c++ ){
    mts = mt[mt_c];
    printf("new mt:::: %d",mts);
 
 LOG_INFO_("...............................................................................");
 LOG_INFO_("..............................NEW RUN..........................................");
 LOG_INFO_("...............................................................................");


/* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
    UDP_SERVER_PORT, udp_rx_callback);
   etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL); //
     
 //int i;
   while((clock_seconds()-ct_start) <= 10) { //count <= 3  //900-1800sec
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
     printf("reachability time:::: %lu\n", clock_seconds());
     uipbuf_set_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS, mts);
 //packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS,3);
 //LOG_INFO("current mt: %d \n",uipbuf_get_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS));
    /* Send to DAG root */
      LOG_INFO("Sending request %d to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
     //abcdefghijklmnopqrstuvwxyzabcdefghij---36
     //abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdef---110
     if (ps[ps_c] == 25) { 
      if (count < 10)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuv00%d", count);
      else if (count < 100)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuv0%d", count);
      else
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuv%d", count);
     }
     else if (ps[ps_c] == 100) {
      if (count < 10)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs00%d", count);
      else if (count < 100)
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs0%d", count);
      else
        snprintf(str, sizeof(str), "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrs%d", count);
     }
   
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
  //  cfs_write (fp, str, sizeof(str));

      count++;
     //if ((clock_seconds() - ct_start) > 3000) {
      //printf("clock difference abrupt: %lu\n", (clock_seconds() - ct_start));
      //break;
     //}
     //else if (count == 301) {
     //  ct_end = clock_seconds();
     // printf("clock difference: %lu\n", (ct_end - ct_start));
    // }
    } else if ((clock_seconds()-ct_start) <= 10){
      LOG_INFO("Not reachable yet\n");
    } else break;

  /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  } //while ends here
    i = i+1;
    //PROCESS_RESTART();
  }//mt for ends here
  i = i+1;
  //if (i <=6)
    //PROCESS_RESTART();
  }//iat for ends here
   i = i+1;
  // PROCESS_RESTART();
  }//ps for ends here
   i = i+1;
//   PROCESS_RESTART();
  }//tp for ends here
  PROCESS_END();

   printf("%d end time: %lu\n",i, clock_seconds());
}
/*---------------------------------------------------------------------------*/
