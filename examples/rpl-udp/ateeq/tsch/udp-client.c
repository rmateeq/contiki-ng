//<<my includes>>
#include "cc2538-rf.h"
#include "dev/radio.h"
#include <stdio.h>
#include <stdlib.h>
#include "sys/energest.h"
#include "net/mac/tsch/tsch.h"
//#include "sys/node-id.h"
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
static unsigned long ct_reach_total = 0;//, ct_unreach_total = 0;
static struct etimer reset_timer;
//unsigned long ct_end;
static int tp[3] = {-3,1,5}; //{7,3,0};//,-3}; //
static int ps[2] = {75,50};//,30};//,35}; //[25,50,75,100];
static int mt[2] = {2,6}; //[1,2,3,4,5];
//bidirectional:yes,no
static int iat[3] = {2,6,12};
static int tp_c = 0;
static int ps_c = 0;
static int mt_c = 0;
static int iat_c = 0;
//static int mts = 0;
static int SEND_INTERVAL = 0;
static int run_time = 600; //600
static int conf_num = 1;
static int REACH = 0;
static int counter = 0;
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

char* constructPacket(
  int packSize,
  unsigned long networkUptime,
  int count
) {
  pack = NULL;
  const char pads[] = "a quick brown fox jumps over the lazy dog.a quick brown fox jumps over the lazy dog.";
  printf("pktleninput: %d",packSize);
  pack = (char*)malloc((packSize+1) * sizeof(char));
  //printf("\n\n%s,,%d,,%d\n\n",pack,strlen(pack),sizeof(pack));
  char *countBuffer = (char*)malloc(5 * sizeof(char));
  //printf("pkt1: %s",pack);
  const int networkUptimeLen = snprintf(pack, 21, "%lu", networkUptime);
  printf("timelength: %d",networkUptimeLen);
  const int countLen = snprintf(countBuffer, 5, "%d", count);
  printf("countlength: %d",countLen);
  const int paddingLen = packSize - (networkUptimeLen + countLen);
  printf("padlength: %d",paddingLen);
  int i;
  for (i = 0; i < paddingLen; i++) {
    pack[networkUptimeLen + i] = pads[i];//',';
  }
  printf("\npkt and length: %s\n",pack,strlen(pkt));
  for (i = 0; i < countLen; i++) {
    pack[networkUptimeLen + paddingLen + i] = countBuffer[i];
  }
  //pack[networkUptimeLen + paddingLen + i] = '\0';
  //printf("last index: %d",i);
  printf("\npkt3: %s\n",pack);
  free(countBuffer);
  if (pack == NULL) {
    printf("\n ---------------------NULL AGAIN--------------\n");
  }
  return pack;
}
/*---------------------------------------------------------------------------*/

static void set_params() 
{
  // Transmissions power [4 options] 0x00(-24),42(-15),58(-13),62(-11),72(-9),88(-7),91(-5),A1(-3),B0(-1),B6(0),C5(1),D5(3),ED(5),FF(7)
  printf("\n________Configuration Number________%d\n",conf_num);
  int tp_val = -99;
  NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tp[tp_c]);
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tp_val);
  printf("P__TP-%d:-:",tp_val);

  printf("P__PS-%d:-:",ps[ps_c]);

  SEND_INTERVAL = (iat[iat_c] * CLOCK_SECOND);
  printf("P__IAT-%d:-:",iat[iat_c]);

  printf("P__MT-%d:-:",mt[mt_c]);

  //ct_start = clock_seconds();
  //printf("M__RUN %d:-:M__STARTTIME %lu:-:\n",conf_num++, ct_start);
}
/*---------------------------------------------------------------------------*/
static unsigned long to_seconds(uint64_t time)
{
  return (unsigned long)(time / ENERGEST_SECOND);
}
static void log_energy()
{
  energest_flush();

  printf("\nEnergest(s):\n");

  printf("E__CPU %4lu:-:E__LPM %4lu:-:E__DEEPLPM %4lu:-:E__TotalTime %lu\n",
    to_seconds(energest_type_time(ENERGEST_TYPE_CPU)), to_seconds(energest_type_time(ENERGEST_TYPE_LPM)),
    to_seconds(energest_type_time(ENERGEST_TYPE_DEEP_LPM)), to_seconds(ENERGEST_GET_TOTAL_TIME()));
  
  printf("E__RadioLISTEN %4lu:-:E__TRANSMIT %4lu:-:E__OFF %4lu\n",
    to_seconds(energest_type_time(ENERGEST_TYPE_LISTEN)), to_seconds(energest_type_time(ENERGEST_TYPE_TRANSMIT)),
    to_seconds(ENERGEST_GET_TOTAL_TIME() - energest_type_time(ENERGEST_TYPE_TRANSMIT)
        - energest_type_time(ENERGEST_TYPE_LISTEN)));
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
  //static char str[120];
  uip_ipaddr_t dest_ipaddr;

  PROCESS_BEGIN();
  
  //printf("header udp: %d",UIP_IPUDPH_LEN);
  //printf("header ip: %d", UIP_IPH_LEN);
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
    
          /* Initialize UDP connection */
          simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);

          /* 20sec pause before starting each new configuration run */
          etimer_set(&reset_timer, random_rand() % (CLOCK_SECOND*20));
          PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&reset_timer)); 

          /*Note the start time of current run*/
          ct_start = clock_seconds();
          printf("\nM__STARTTIME %lu:-:\n", ct_start);

          /* Initialize timer for send interval */
          etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL); //
     
          while((clock_seconds()-ct_start) <= run_time) 
          {
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

            if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) 
            {
              //REACH = 1
              ct_reach = clock_seconds();
              if (counter == 0)
              {
                printf("\nM__REACHTIME %lu:-:\n", clock_seconds());
              }
              //if ((ct_reach > ct_unreach) && (REACH == 0))
              //{
              //  ct_unreach_total += (ct_reach - ct_unreach);
                REACH = 1;
              //}

              uipbuf_set_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS, mt[mt_c]);
              //LOG_INFO("current mt: %d \n",uipbuf_get_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS));
              /* Send to DAG root */
              printf("D__SEQNO %d:-:\n", counter+1);
              //LOG_INFO("Sending request %d to ", counter);
              //LOG_INFO_6ADDR(&dest_ipaddr);
              //LOG_INFO_("\n");

              const uint64_t network_uptime = tsch_get_network_uptime_ticks();
              char* packet = NULL;
              packet = constructPacket(ps[ps_c], network_uptime, ++counter);
              free(pack);
              //printf("Message sent: %s",packet);
              printf("M__MSGLEN %d",strlen(packet));
              simple_udp_sendto(&udp_conn, packet, strlen(packet), &dest_ipaddr);
              //simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
              //counter++;
            }
            else //if ((clock_seconds()-ct_start) <= sim_time)
            {
              ct_unreach = clock_seconds();
              if ((ct_unreach > ct_reach) && (REACH == 1))
              {
                ct_reach_total += (ct_unreach - ct_reach);
              }
              //if ()
              //{
                REACH = 0;
              //}
              //printf("M__UNREACHTIME %lu:-:\n", clock_seconds());
            } 
            //else break;

            /* Add some jitter */
            etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
          } //while ends here

          log_energy();
          //printf("M__TOTALPKTSSENT-%d:-:",counter);
          //counter = 0;
          printf("M__STARTED-%lu:-:M__ENDED-%lu:-:M__TOTAL-%lu\n", ct_start , clock_seconds() , (clock_seconds()-ct_start));
          printf("M__REACHTIME-%lu\n", ct_reach_total);
          printf("M__CONFNUM-%d ENDS\n<***>\n<***>\n",conf_num++);
        }//mt for ends here
      }//iat for ends here
    }//ps for ends here
  }//tp for ends here
  printf("M__TOTALPKTSSENT-%d:-:",counter);
  printf("\nM__ENDTIME-%lu\n<**********>\n<**********>\n", clock_seconds());

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
