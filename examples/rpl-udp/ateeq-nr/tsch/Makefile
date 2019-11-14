CONTIKI_PROJECT = tsch-client tsch-server
all: $(CONTIKI_PROJECT)

DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI=../../../../

MAKE_MAC = MAKE_MAC_TSCH
#MAKE_MAC = MAKE_MAC_CSMA
MAKE_NET = MAKE_NET_NULLNET

include $(CONTIKI)/Makefile.include
