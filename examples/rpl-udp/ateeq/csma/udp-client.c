//<<my includes>>
#include "cc2538-rf.h"
#include "dev/radio.h"
#include <stdio.h>
#include <stdlib.h>
#include "sys/energest.h"
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

//<<my vars>>
static unsigned long ct_start, ct_reach, ct_unreach;
static unsigned long ct_reach_total = 0;
static struct etimer reset_timer;
//run separately for each power level
static int tp[1] = {1};//{5,3,1,-1};//{7,5,3,1,-1}; 
static int ps[3] = {27,52,77};
static int mt[3] = {8,4,2}; 

#define DENSITY 1 //2,3
#if DENSITY == 1
static float iat[4] = {4,3,2,1};
#elif DENSITY == 2
static float iat[4] = {8,6,4,2};
#elif DENSITY == 3
static float iat[4] = {12,9,6,3};
#endif

static int tp_c = 0;
static int ps_c = 0;
static int mt_c = 0;
static int iat_c = 0;
static float SEND_INTERVAL = 0;
static int run_time = 600;
static int conf_num = 1;
static int REACH = 0;
static int counter = 0;
const int run_delay = 15;
int local_counter = 0;
char* pack = NULL;
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

char* constructPacket(int packSize, unsigned long networkUptime, int count)
{
  pack = NULL;
  const char pads[] = "a quick brown fox jumps over the lazy dog.a quick brown fox jumps over the lazy dog.";
  pack = (char*)malloc((packSize+1) * sizeof(char));
  char *countBuffer = (char*)malloc(5 * sizeof(char));
  const int networkUptimeLen = snprintf(pack, 21, "%lu", networkUptime);
  const int countLen = snprintf(countBuffer, 5, "%d", count);
  const int paddingLen = packSize - (networkUptimeLen + countLen);
  
  int i;
  for (i = 0; i < paddingLen; i++) {
    pack[networkUptimeLen + i] = pads[i];//',';
  }
  for (i = 0; i < countLen; i++) {
    pack[networkUptimeLen + paddingLen + i] = countBuffer[i];
  }
  pack[networkUptimeLen + paddingLen + i] = '\0';

  free(countBuffer);

  if (pack == NULL) {
    printf("\n ---------------------NULL AGAIN--------------\n");
  }
  return pack;
}
/*---------------------------------------------------------------------------*/

static void set_params() 
{
  printf("\n\n_______________Configuration Number_______________%d\n\n",conf_num);
  int tp_val = -99;
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp[tp_c]);
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  printf("P__TP,%d:-:",tp_val);
  printf("P__PS,%d:-:",ps[ps_c]);
  SEND_INTERVAL = (iat[iat_c] * CLOCK_SECOND);
  //printf("P__IAT,%f:-:",iat[iat_c]);
  printf("P__IAT,%d:-:",(int)iat[iat_c]);
  //printf("interval,%f",SEND_INTERVAL);
  printf("P__MT,%d:-:",mt[mt_c]);
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
 uint16_t sender_port, const uip_ipaddr_t *receiver_addr, uint16_t receiver_port,
 const uint8_t *data, uint16_t datalen)
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
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN(); 
  
  for (tp_c = 0; tp_c < (sizeof(tp) / sizeof(tp[0])); tp_c++ )
  {  
    for (ps_c = 0; ps_c < (sizeof(ps) / sizeof(ps[0])); ps_c++ )
    { 
      for (iat_c = 0; iat_c < (sizeof(iat) / sizeof(iat[0])); iat_c++ )
      {
        for (mt_c = 0; mt_c < (sizeof(mt) / sizeof(mt[0])); mt_c++ )
        {          
          //set parameter configuration
          set_params();
          
          //etimer_set(&reset_timer, random_rand() % (CLOCK_SECOND*3));
          //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));

          //NETSTACK_MAC.on();
          
          //etimer_set(&reset_timer, random_rand() % (CLOCK_SECOND*7));
          //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer));
          
          //NETSTACK_ROUTING.global_repair("simple reset");
    
          /* Initialize UDP connection */
          simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);
          //if (counter == 0)
          //{
          //  /* 20sec pause before starting each new configuration run */
            etimer_set(&reset_timer, (CLOCK_SECOND*run_delay));
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer)); 
          //}

          //NETSTACK_MAC.on();
          /*Note the start time of current run*/
          ct_start = clock_seconds();
          printf("\nM__STARTTIME,%lu:-:", ct_start);
          
          /* Initialize timer for send interval */
          etimer_set(&periodic_timer, random_rand() % (int) SEND_INTERVAL); //
          
          while((clock_seconds()-ct_start) <= run_time) 
          {
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

            if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) 
            {
              if (local_counter == 0)
              {
                ct_reach = clock_seconds();
                printf("M__FIRSTRUNREACHTIME,%lu:-:", clock_seconds());
              }
              else if (REACH == 0)
                ct_reach = clock_seconds();
              REACH = 1;
              
              uipbuf_set_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS, mt[mt_c]);
              
              printf("\nD__SEQNO,%d:-:", counter+1);
              //LOG_INFO_6ADDR(&dest_ipaddr);
              
              //const uint64_t network_uptime = clock_second()*CLOCK_SECOND; //;tsch_get_network_uptime_ticks();
              char* packet = NULL;
              //(unsigned long) ((double) RTIMER_NOW()*(0.030518)
              packet = constructPacket(ps[ps_c], (unsigned long) (clock_seconds()*CLOCK_SECOND), ++counter);
              free(pack);
              //printf("Message sent: %s",packet);
              //printf("M__MSGLEN %d",strlen(packet));
              simple_udp_sendto(&udp_conn, packet, strlen(packet), &dest_ipaddr);
              
              local_counter++;
            }
            else
            {
              ct_unreach = clock_seconds();
              if ((ct_unreach > ct_reach) && (REACH == 1))
              {
                ct_reach_total += (ct_unreach - ct_reach);
              }
              REACH = 0;
            }
            /* Add some jitter */
            etimer_set(&periodic_timer, (int) SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * (int) CLOCK_SECOND)));
          } //while ends header
          //flush energy stats
          log_energy();

          //turn off mac and network
          //NETSTACK_RADIO.off()
          //NETSTACK_MAC.off();
          //NETSTACK_MAC.init();
          //NETSTACK_MAC.on();
          //NETSTACK_NETWORK.off();
          //printf("M__TOTALPKTSSENT-%d:-:",counter);
          //counter = 0;
          printf("M__RUNENDED,%lu:-:M__RUNTIME,%lu:-:M__RUNREACHABLETIME,%lu\n",clock_seconds() , (clock_seconds()-ct_start), ct_reach_total);
          printf("M__RUNPKTSSENT,%d:-:",local_counter);
          printf("M__RUNCONFNUM,%d ENDS\n<***>\n<***>\n",conf_num++);
          local_counter = 0;
        }//mt for ends here
      }//iat for ends here
    }//ps for ends here
  }//tp for ends here
  printf("M__TOTALPKTSSENT,%d:-:",counter);
  printf("\nM__ENDTIME,%lu\n<**********>\n<**********>\n", clock_seconds());

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
