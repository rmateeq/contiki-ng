//<<my includes>>
#include "cc1200-const.h"
#include "cc1200-conf.h"
#include "cc1200-arch.h"
#include "cc1200-rf-cfg.h"
#include "dev/radio.h"
#include "node-id.h"
//>>my includes<<

#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
//<<set interval 10 instead of 60>>
#define SEND_INTERVAL		  (1 * CLOCK_SECOND)
 //>>set interval 10 instead of 60<<
//<<my vars>>
// int ps[4] = {};
// int iat[5] = {};
// int tp[4] = {};
// int mt[4] = {};
// 
//>>my vars<<
static struct simple_udp_connection udp_conn;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
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

  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");

}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static unsigned count;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;
  FILE * fp NULL;
 
  PROCESS_BEGIN();

  //<<file>>
  
  //int i;
  const char filepath[] = "/groups/wall2../rt-mac/data/";
  //const char nodeid[] = ;
  char sbuf[1024];
  sprintf (sbuf, "%s/%d", filepath, node_id);
  fp = fopen (sbuf,"a");
  //>>file<<
 
  //<<set tx power>>
  //printf("tp before %d \n",RADIO_PARAM_TXPOWER);
 int val1;
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &val1);
  printf("....tp before %d \n",val1);
  //radio_value_t tx_level = -16;
  //int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, CC1200_CONST_TX_POWER_MIN);
  int rd = NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, 0);
  //update_txpower((int8_t)-16);
  printf("tp state %d \n",rd);
  printf("tp after %d \n",RADIO_PARAM_TXPOWER);
  int val;
  NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &val);
  printf("....tp after %d \n",val);
  //>>set tx power<<
         
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
 //<<set tx power>>
  //printf("tp after %d \n",RADIO_PARAM_TXPOWER);
  //>>set tx power<<
  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
      /* Send to DAG root */
      LOG_INFO("Sending request %u to ", count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      snprintf(str, sizeof(str), "hello %d", count);
     //<<set tx power>>
     //printf("tp after %d \n",RADIO_PARAM_TXPOWER);
  //>>set tx power<<
      simple_udp_sendto(&udp_conn, str, strlen(str), &dest_ipaddr);
      fprintf (fp, str);
 
      count++;
    } else {
      LOG_INFO("Not reachable yet\n");
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }
    /* close the file*/  
   fclose (fp);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
