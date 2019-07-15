#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;
static unsigned long ct_start;
int sim_time = 600;
static int tp[4] = {7,3,0,-3};
static int tp_index = 0;

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
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
  //LOG_INFO("Received request '%.*s' from ", datalen, (char *) data);
  printf("Received request '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
  printf("\n");
  //LOG_INFO_("\n");
  int lqi_val;
  int rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi_val);
  printf("lqi-state: %d::",rd);
  printf("lqi: %d::",lqi_val);
  int rssi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi_val); //RADIO_PARAM_LAST_RSSI, RADIO_PARAM_LAST_PACKET_TIMESTAMP
  printf("rssi-state: %d::",rd);
  printf("rssi: %d::\n",rssi_val);
         
  if (clock_seconds() - ct_start >= sim_time){
  int tp_val = tp[tp_index++];
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  printf("tp state server:::: %d\n",rd);
  printf("changed tp of server to:::: %d\n",tp_val);
  ct_start = clock_seconds();
  }
         
#if WITH_SERVER_REPLY
  /* send back the same string to the client as an echo reply */
  LOG_INFO("Sending response.\n");
  simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
#endif /* WITH_SERVER_REPLY */
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();
  ct_start = clock_seconds();
  printf("start time: %lu\n", ct_start);
  int tp_val = -99;
  //int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp[tp_c]);
  int rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
   LOG_INFO("tp state server:::: %d\n",rd);
   LOG_INFO("current tp server:::: %d\n",tp_val);
  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
