#include "contiki.h"
#include "net/netstack.h"
#include "net/nullnet/nullnet.h"
#include "sys/energest.h"
#include <string.h>
#include <stdio.h> /* For printf() */

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

/* Configuration */
#define SEND_INTERVAL (8 * CLOCK_SECOND)
static linkaddr_t dest_addr =         {{ 0x00, 0x12, 0x4b, 0x00, 0x10, 0x03, 0x56, 0x2d }};

#if MAC_CONF_WITH_TSCH
#include "net/mac/tsch/tsch.h"
static linkaddr_t coordinator_addr =  {{ 0x00, 0x12, 0x4b, 0x00, 0x10, 0x03, 0x56, 0x2d }};
#endif /* MAC_CONF_WITH_TSCH */

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

#if MAC_CONF_WITH_TSCH
  tsch_set_coordinator(linkaddr_cmp(&coordinator_addr, &linkaddr_node_addr));
#endif /* MAC_CONF_WITH_TSCH */

  /* Initialize NullNet */
  nullnet_buf = (uint8_t *)&count;
  nullnet_len = sizeof(count);
  nullnet_set_input_callback(input_callback);

  if(!linkaddr_cmp(&dest_addr, &linkaddr_node_addr)) {
    etimer_set(&periodic_timer, SEND_INTERVAL);
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
      LOG_INFO("Sending %u to ", count);
      LOG_INFO_LLADDR(&dest_addr);
      LOG_INFO_("\n");

      NETSTACK_NETWORK.output(&dest_addr);
      count++;
      etimer_reset(&periodic_timer);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
