#include "contiki.h"
#include "net/linkaddr.h"
#include <stdio.h> /* For printf() */

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

/*---------------------------------------------------------------------------*/
PROCESS(eui64_retriever_process, "EUI64 Retriever");
AUTOSTART_PROCESSES(&eui64_retriever_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(eui64_retriever_process, ev, data)
{
  static struct etimer timer;

  PROCESS_BEGIN();

  /* Setup a periodic timer that expires after 10 seconds. */
  etimer_set(&timer, CLOCK_SECOND * 10);
  //printf("I am in..");
  while(1) {
   print_lladdr(&linkaddr_node_addr);
   printf("\n");

    /* Wait for the periodic timer to expire and then restart the timer. */
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    etimer_reset(&timer);
  }

  PROCESS_END();
}
