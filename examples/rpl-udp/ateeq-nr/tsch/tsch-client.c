//<<my includes>>
#include "cc2538-rf.h"
#include "dev/radio.h"
#include <stdio.h>
#include <stdlib.h>
#include "sys/energest.h"
#include "net/mac/tsch/tsch.h"
#include "net/nullnet/nullnet.h"
//>>my includes<<

#include "contiki.h"
#include "random.h"
#include "net/netstack.h"
#include "sys/log.h"

#ifndef LOG_MODULE
#define LOG_MODULE "App"
#endif
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

//<<my vars>>
static unsigned long ct_start;
//run separately for each power level
static int tp[1] = {3};//{-3}; 
static int ps[3] = {27,52,77};
static int mt[3] = {8,4,2}; 
//bidirectional:yes,no

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
static int counter = 0;
const int run_delay = 30;
int local_counter = 0;
char* pack = NULL;
//>>my vars<<

/*---------------------------------------------------------------------------*/
PROCESS(tsch_client_process, "TSCH client");
AUTOSTART_PROCESSES(&tsch_client_process);
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
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(tsch_client_process, ev, data)
{
  static struct etimer periodic_timer;
  PROCESS_BEGIN(); 
  int tpval = tp[0];
  int rv = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tpval);
  rv = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tpval);
  printf("NEWTPVAL,%dSUCCESS,%d",tpval,rv);
  /* Initialize UDP connection */
  
  
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
          
          /*Note the start time of current run*/
          ct_start = clock_seconds();
          printf("\nM__STARTTIME,%lu:-:", ct_start);
          
          /* Initialize timer for send interval */
          etimer_set(&periodic_timer, random_rand() % (int) SEND_INTERVAL); //
         
          while((clock_seconds()-ct_start) <= run_time) 
          {
           if (tsch_is_associated) {   
              uipbuf_set_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS, mt[mt_c]);
              
              printf("\nD__SEQNO,%d:-:", counter+1);
              //LOG_INFO_6ADDR(&dest_ipaddr);
              
              //const uint64_t network_uptime = tsch_get_network_uptime_ticks();
              char* packet = NULL;
              packet = constructPacket(ps[ps_c], tsch_get_network_uptime_ticks(), ++counter);
              free(pack);
              //printf("Message sent: %s",packet);
              //printf("M__MSGLEN %d",strlen(packet));
              //uint8_t payload[64] = { 0 };
              nullnet_buf = (uint8_t *) packet; /* Point NullNet buffer to 'payload' */
              nullnet_len = ps[ps_c]; /* Tell NullNet that the payload length is two bytes */
              NETSTACK_NETWORK.output(NULL); /* Send as broadcast */            
              local_counter++;
            /* Add some jitter */
            etimer_set(&periodic_timer, (int) SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * (int) CLOCK_SECOND)));
           }
          } //while ends header
          //flush energy stats
          log_energy();

          //turn off mac and network
          //NETSTACK_RADIO.off()
          NETSTACK_MAC.off();
          NETSTACK_MAC.init();
          NETSTACK_MAC.on();
          //NETSTACK_NETWORK.off();
          //printf("M__TOTALPKTSSENT-%d:-:",counter);
          //counter = 0;
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
