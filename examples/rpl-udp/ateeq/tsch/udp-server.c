#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/mac/tsch/tsch.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uiplib.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  0
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;
static unsigned long ct_start;
static struct etimer reset_timer;
//static int conf_num = 48;
static int tp[4] = {7,3,0,-3}; //[-13,-9,-5,-1,1,3,5];
//static int ps[2] = {25,100}; //[25,50,75,100];
//static int mt[2] = {5,1}; //[1,2,3,4,5];
//bidirectional:yes,no
//static int iat[3] = {10,6,2}; //[1,2,4,6,8,10];
static int tp_c = 0;
static int ps_c = 0;
static int mt_c = 0;
static int iat_c = 0;
//static int mts = 0;
static int run_time = 15; //600
//static int conf_num = 1;


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
  const int countExtracted = extractCount(datalen, (char *) data);
  //if(datalen >= sizeof(remote_time_clock_ticks)) {
  //  memcpy(&remote_time_clock_ticks, data, sizeof(remote_time_clock_ticks));

    printf("\nD__SEQNO-%d:-:",countExtracted);
    char buf[UIPLIB_IPV6_MAX_STR_LEN];
    uiplib_ipaddr_snprint(buf, sizeof(buf), sender_addr);
    printf("%s", buf);
    //LOG_INFO_6ADDR(sender_addr);
    printf("M__CREATETIME-%lu:-:, M__CURRENTTIME-%lu:-:, M__DELAY-%lu:-:",
              (unsigned long)remote_time_clock_ticks,
              (unsigned long)local_time_clock_ticks,
              (unsigned long)(local_time_clock_ticks - remote_time_clock_ticks));

  int lqi_val;
  int rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi_val);
  printf("M__LQISTATE-%d:-:D__LQI-%d:-:",rd,lqi_val);
  //printf("lqi: %d::",lqi_val);
  int rssi_val;
  rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi_val); //RADIO_PARAM_LAST_RSSI, RADIO_PARAM_LAST_PACKET_TIMESTAMP
  printf("M__RSSISTATE-%d:-:D__RSSI-%d",rd,rssi_val);
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
  ct_start = clock_seconds();
  printf("start time: %lu\n", ct_start);

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
  
  for (tp_c = 0; tp_c <= 3; tp_c++ )
  { 
    printf("after udp register\n");
    int tp_val = tp[tp_c];
    int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp_val);
    rd = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
    printf("P__TP-%d:-:\n",tp_val);
    printf("M__TPSTATE-%d:-:M__TPSETTIME-%lu\n",rd,clock_seconds());
    
    //ct_start = clock_seconds();

    etimer_set(&reset_timer, run_time);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
    printf("after tp set at %lu\n",clock_seconds());
    //etimer_set(&reset_timer, run_time); 
  
    for (ps_c = 0; ps_c <= 1; ps_c++ )
    { 
      etimer_set(&reset_timer, run_time);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
      //etimer_set(&reset_timer, run_time);

      for (iat_c = 0; iat_c <= 2; iat_c++ )
      {
        etimer_set(&reset_timer, run_time);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
        //etimer_set(&reset_timer, run_time);
  
        for (mt_c = 0; mt_c <= 1; mt_c++ )
        {
          etimer_set(&reset_timer, run_time);
          PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
          //etimer_set(&reset_timer, run_time); 
        }
      }
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
