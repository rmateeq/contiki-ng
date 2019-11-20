#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "sys/energest.h"
#include <string.h>
#include <stdio.h> /* For printf() */
#include <stdlib.h>
#include "random.h"
/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
//#define SEND_INTERVAL (8 * CLOCK_SECOND)
static linkaddr_t dest_addr =         {{ 0x00, 0x12, 0x4b, 0x00, 0x10, 0x03, 0x56, 0x2d }};

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x10, 0x03, 0x56, 0x2d }};
#endif /* MAC_CONF_WITH_TSCH */

//<<my vars>>
static unsigned long ct_start;
//run separately for each power level
static int tp[1] = {7};//{-3}; 
static int ps[3] = {27,52,77};
static int mt[3] = {6,4,2}; 
//bidirectional:yes,no
//static linkaddr_t dest_addr =         {{ 0x00, 0x12, 0x4b, 0x00, 0x10, 0x03, 0x56, 0x2d }};
#define DENSITY 1 //2,3
#if DENSITY == 1
static float iat[4] = {12,9,6,3};//{4,3,2,1};
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

/*-----------------------------------------------------------------------------
----------------------------SERVER FUNCTIONS START-----------------------------
-----------------------------------------------------------------------------*/
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
void input_callback(const void *data, uint16_t datalen,
  const linkaddr_t *src, const linkaddr_t *dest)
{
    if(datalen == sizeof(unsigned)) {
    unsigned count;
    memcpy(&count, data, sizeof(count));
    LOG_INFO("---------------Received %u from ", count);
    LOG_INFO_LLADDR(src);
    LOG_INFO("\nAt");
    LOG_INFO_LLADDR(dest);
    LOG_INFO_("\n");
  }


  // printf("packet received\n\n");
  // uint64_t local_time_clock_ticks = tsch_get_network_uptime_ticks();
  // uint64_t remote_time_clock_ticks = extractNetworkUptime(datalen, (char *) data);
  // const int countExtracted = extractCount(datalen, (char *) data);
  // counter++;
  // //per_conf_counter++;
  // printf("\nD__SEQNO,%d:-:",countExtracted);
  
  // LOG_INFO_LLADDR(src);
  // LOG_INFO_LLADDR(dest);
  
  // //LOG_INFO_6ADDR(sender_addr);
  // printf("\nM__CREATETIME,%lu:-:M__CURRENTTIME,%lu:-:M__DELAY,%lu",
  //           (unsigned long)remote_time_clock_ticks, (unsigned long)local_time_clock_ticks,
  //           (unsigned long)(local_time_clock_ticks - remote_time_clock_ticks));

  // int lqi_val;
  // int rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_LINK_QUALITY, &lqi_val);
  // printf("\nM__LQISTATE,%d:-:M__LQI,%d:-:",rd,lqi_val);
  
  // int rssi_val;
  // rd = NETSTACK_RADIO.get_value(RADIO_PARAM_LAST_RSSI, &rssi_val);
  // printf("M__RSSISTATE,%d:-:M__RSSI,%d\n",rd,rssi_val);
  //printf("rssi: %d::\n",rssi_val);
}
/*---------------------------------------------------------------------------*/
void
print_lladdr(const linkaddr_t *lladdr)
{
  if(lladdr == NULL) {
     printf("(NULL LL addr)");
     return;
  } else {
    unsigned int i;
    for(i = 0; i < LINKADDR_SIZE; i++) {
       if(i > 0) {
            printf("-");
       }
       printf("%02x", lladdr->u8[i]);
    }
  }
}
/*-----------------------------------------------------------------------------
-----------------------------SERVER FUNCTIONS END------------------------------
-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
----------------------------CLIENT FUNCTIONS START-----------------------------
-----------------------------------------------------------------------------*/

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

/*-----------------------------------------------------------------------------
-----------------------------CLIENT FUNCTIONS END------------------------------
-----------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
PROCESS(nrpu_process, "NRPU");
AUTOSTART_PROCESSES(&nrpu_process);

/*---------------------------------------------------------------------------*/
/*void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  if(len == sizeof(unsigned)) {
    unsigned count;
    memcpy(&count, data, sizeof(count));
    LOG_INFO("Received %u from ", count);
    LOG_INFO_LLADDR(src);
    LOG_INFO_("\n");
  }
}*/
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nrpu_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count = 0;

  PROCESS_BEGIN();
  int tpval = tp[0];
  int rv = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, tpval);
  rv = NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &tpval);
  printf("NEWTPVAL,%dSUCCESS,%d",tpval,rv);

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  if(!linkaddr_cmp(&dest_addr, &linkaddr_node_addr)) {

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
              //uipbuf_set_attr(UIPBUF_ATTR_MAX_MAC_TRANSMISSIONS, mt[mt_c]);
            //TSCH_MAC_MAX_FRAME_RETRIES = mt[mt_c];
            packetbuf_set_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS,mt[mt_c]);
            printf("max tx set to %d;;;",TSCH_MAC_MAX_FRAME_RETRIES);
              
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
              NETSTACK_NETWORK.output(&dest_addr); /* Send as broadcast */            
              local_counter++;
            /* Add some jitter */
            etimer_set(&periodic_timer, (int) SEND_INTERVAL - CLOCK_SECOND + (random_rand() % (2 * (int) CLOCK_SECOND)));
          } //while ends header
          log_energy();

          printf("M__RUNPKTSSENT,%d:-:",local_counter);
          printf("M__RUNCONFNUM,%d ENDS\n<***>\n<***>\n",conf_num++);
          local_counter = 0;
        }//mt for ends here
      }//iat for ends here
    }//ps for ends here
  }//tp for ends here
  printf("M__TOTALPKTSSENT,%d:-:",counter);
  printf("\nM__ENDTIME,%lu\n<**********>\n<**********>\n", clock_seconds());

/*    etimer_set(&periodic_timer, SEND_INTERVAL);
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      LOG_INFO("Sending %u to ", count);
      LOG_INFO_LLADDR(&dest_addr);
      LOG_INFO_("\n");

      NETSTACK_NETWORK.output(&dest_addr);
      count++;
      etimer_reset(&periodic_timer);
    } */
   
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
