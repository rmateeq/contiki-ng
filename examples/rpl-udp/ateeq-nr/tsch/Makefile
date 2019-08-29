CONTIKI_PROJECT = udp-client udp-server
all: $(CONTIKI_PROJECT)

DEFINES+=PROJECT_CONF_H=\"project-conf.h\"

CONTIKI=../../../../

MAKE_MAC = MAKE_MAC_TSCH
#MAKE_MAC = MAKE_MAC_CSMA

include $(CONTIKI)/Makefile.include
