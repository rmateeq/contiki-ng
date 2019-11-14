#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/mac/tsch/tsch.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uiplib.h"
#include "random.h"
#include "sys/energest.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn;
static int counter = 0;
int tp_val = 3;
PROCESS(udp_server_process, "UDP server");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/

unsigned long extractNetworkUptime(
  int packSize,
  char* packet
) {
  int i;
  for (i = 0; i < packSize; i++) {
    if( (packet[i] >='a' && packet[i] <='z') || (packet[i] == ' ') || (packet[i] == '.')) {
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
    if( (packet[i] >='a' && packet[i] <='z') || (packet[i] == ' ') || (packet[i] == '.')) {
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
udp_rx_callback(struct simple_udp_connection *c, const uip_ipaddr_t *sender_addr,
         uint16_t sender_port, const uip_ipaddr_t *receiver_addr, 
         uint16_t receiver_port, const uint8_t *data, uint16_t datalen)
{
  
  uint64_t local_time_clock_ticks = tsch_get_network_uptime_ticks();
  uint64_t remote_time_clock_ticks = extractNetworkUptime(datalen, (char *) data);
  const int countExtracted = extractCount(datalen, (char *) data);
  counter++;
  //per_conf_counter++;
  printf("\nD__SEQNO,%d:-:",countExtracted);
  char buf[UIPLIB_IPV6_MAX_STR_LEN];
  uiplib_ipaddr_snprint(buf, sizeof(buf), sender_addr);
  printf("%s", buf);
  //LOG_INFO_6ADDR(sender_addr);
  printf("\nM__CREATETIME,%lu:-:M__CURRENTTIME,%lu:-:M__DELAY,%lu",
            (unsigned long)remote_time_clock_ticks, (unsigned long)local_time_clock_ticks,
            (unsigned long)(local_time_clock_ticks - remote_time_clock_ticks));

  int lqi_val;
  int rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi_val);
  printf("\nM__LQISTATE,%d:-:M__LQI,%d:-:",rd,lqi_val);
  
  int rssi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi_val);
  printf("M__RSSISTATE,%d:-:M__RSSI,%d\n",rd,rssi_val);
  //printf("rssi: %d::\n",rssi_val);
         
         
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
    tsch_set_coordinator(1);
    int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
    rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
    printf("\nP__TP,%d:-:M__TPSTATE,%d:-:M__TPSETTIME,%lu\n",tp_val,rd,clock_seconds());
      printf("M__STARTTIME,%lu\n", clock_seconds());
      /* Initialize UDP connection */
      simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                          UDP_CLIENT_PORT, udp_rx_callback);  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
