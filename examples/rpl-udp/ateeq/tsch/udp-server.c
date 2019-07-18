#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/mac/tsch/tsch.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;
static unsigned long ct_start;
//static int conf_num = 48;
int run_time = 10;
static int tp[4] = {7,3,0,-3};
static int tp_index = 0;

PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/
unsigned long extractNetworkUptime(
  int packSize,
  char* packet
) {
  int i;
  for (i = 0; i < packSize; i++) {
    if (packet[i] == ',') {
      break;
    }
  }

  const int upperBound = i;
  unsigned long networkUptime = 0UL;

  for (i = 0; i < upperBound; i++) {
    networkUptime = networkUptime * 10 + packet[i] - '0';
  }

  return networkUptime;
}
/*---------------------------------------------------------------------------*/
int extractCount(
  int packSize,
  char* packet
) {
  int i;
  for (i = packSize - 1; i > 0; i--) {
    if (packet[i] == ',') {
      break;
    }
  }

  int count = 0;
  for (i++; i < packSize; i++) {
    count = count * 10 + packet[i] - '0';
  }

  return count;
}
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
  //printf("Received request '%.*s' from ", datalen, (char *) data);
  //LOG_INFO_6ADDR(sender_addr);
  printf("\n");

  //const unsigned long networkUptimeExtracted = extractNetworkUptime(packSize, packet);



  uint64_t local_time_clock_ticks = tsch_get_network_uptime_ticks();
  uint64_t remote_time_clock_ticks = extractNetworkUptime(datalen, (char *) data);
  const int countExtracted = extractCount(packSize, packet);
  //if(datalen >= sizeof(remote_time_clock_ticks)) {
  //  memcpy(&remote_time_clock_ticks, data, sizeof(remote_time_clock_ticks));

    printf("D__SEQNO-%d:-:",countExtracted);
    char buf[UIPLIB_IPV6_MAX_STR_LEN];
    uiplib_ipaddr_snprint(buf, sizeof(buf), sender_addr);
    printf("%s", buf);
    //LOG_INFO_6ADDR(sender_addr);
    printf(", created at %lu, now %lu, latency %lu clock ticks\n",
              (unsigned long)remote_time_clock_ticks,
              (unsigned long)local_time_clock_ticks,
              (unsigned long)(local_time_clock_ticks - remote_time_clock_ticks));



  //LOG_INFO_("\n");
  int lqi_val;
  int rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi_val);
  printf("lqi-state: %d::",rd);
  printf("lqi: %d::",lqi_val);
  int rssi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi_val); //RADIO_PARAM_LAST_RSSI, RADIO_PARAM_LAST_PACKET_TIMESTAMP
  printf("rssi-state: %d::",rd);
  printf("rssi: %d::\n",rssi_val);
         
  if (clock_seconds() - ct_start >= run_time){
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

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
  
  while (1) {
    if (clock_seconds() - ct_start >= run_time){
      printf("after udp register\n");
      int tp_val = tp[tp_index++];
      int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
      rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
      printf("tp state server:::: %d\n",rd);
      printf("changed tp of server to:::: %d\n",tp_val);
      ct_start = clock_seconds();
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
