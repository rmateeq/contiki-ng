#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
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
//static unsigned long ct_start;
static struct etimer reset_timer;
//static int conf_num = 48;
//int tp[4] = {5,3,1,-1};//{7,5,3,1,-1};
//int tp_c = 0;
int run_time = 650;
const int num_conf = 36;
static int counter = 0;
int tp_val = -5;
//int per_conf_counter = 0;
const int run_delay = 15;
//int skew_pad = 0;
static struct etimer reset_timer;
//int conf_count = 1;
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
static unsigned long to_seconds(uint64_t time)
{
  return (unsigned long)(time / ENERGEST_SECOND);
}
/*---------------------------------------------------------------------------*/

static void log_energy()
{
  energest_flush();

  printf("\nM__RUNEnergest(s):\n");

  printf("E__RCPU,%4lu:-:E__LPM,%4lu:-:E__DEEPLPM,%4lu:-:E__TotalTime,%lu\n",
    to_seconds(energest_type_time(ENERGEST_TYPE_CPU)), to_seconds(energest_type_time(ENERGEST_TYPE_LPM)),
    to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)), to_seconds(ENERGEST_GET_TOTAL_TIME()));
  
  printf("E__RadioLISTEN,%4lu:-:E__TRANSMIT,%4lu:-:E__OFF,%4lu\n",
    to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)), to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
    to_seconds(ENERGEST_GET_TOTAL_TIME() - energest_type_time(ENERGEST_TYPE_TRANSMIT)
        - energest_type_time(ENERGEST_TYPE_LISTEN)));
}
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c, const uip_ipaddr_t *sender_addr,
         uint16_t sender_port, const uip_ipaddr_t *receiver_addr, 
         uint16_t receiver_port, const uint8_t *data, uint16_t datalen)
{  
  uint64_t local_time_clock_ticks = clock_seconds()*CLOCK_SECOND;//(unsigned long) ((double) RTIMER_NOW()*(0.030518));//20UL; //tsch_get_network_uptime_ticks();
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
  
  int lts_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_PACKET_TIMESTAMP, &lts_val);
  printf("\nM__LTSSTATE,%d:-:M__LTS,%d:-:",rd,lts_val);
         
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
  printf("M__CLOCKTIMET_START,%lu\n", clock_time());
  
//  for (tp_c = 0; tp_c < (sizeof(tp) / sizeof(tp[0])); tp_c++ )
//  { 
    //printf("after udp register\n");
//    int tp_val = tp[tp_c];
    int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
    rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
    printf("\nP__TP,%d:-:M__TPSTATE,%d:-:M__TPSETTIME,%lu\n",tp_val,rd,clock_seconds());
    //i = 1;
//    while (conf_count <= num_conf)
//    {
//      printf("\n\n_______________Configuration Number_______________%d\n\n",conf_count+(tp_c*num_conf));
      
      //etimer_set(&reset_timer, random_rand() % (CLOCK_SECOND*3));
      //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
      //NETSTACK_MAC.on();
      //etimer_set(&reset_timer, random_rand() % (CLOCK_SECOND*7));
      //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
    /* Initialize DAG root */
      NETSTACK_ROUTING.root_start();
      //per_conf_counter = 0;
//      etimer_set(&reset_timer, ((CLOCK_SECOND*run_delay)+skew_pad));
      etimer_set(&reset_timer, (CLOCK_SECOND*run_delay));
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
      //ct_start = clock_seconds();
      printf("M__STARTTIME,%lu\n", clock_seconds());
      /* Initialize UDP connection */
      simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                          UDP_CLIENT_PORT, udp_rx_callback);
  
      etimer_set(&reset_timer, (run_time*CLOCK_SECOND*num_conf));
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
//      printf("\nM__PKTSRECVD,%d:-:CONFNUM,%d\n",per_conf_counter,conf_count+(tp_c*num_conf));
//      per_conf_counter = 0;
      //NETSTACK_MAC.off();
//      conf_count += 1;
//      skew_pad += 8;
      log_energy();
      //NETSTACK_MAC.init();
      //NETSTACK_MAC.on();
      //tsch_set_coordinator(1);
//    }
//    conf_count = 1;
//  }
  
  printf("M__TOTALPKTSRECVD,%d:-:",counter);
  printf("\nM__ENDTIME,%lu\n<**********>\n<**********>\n", clock_seconds());
  
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
