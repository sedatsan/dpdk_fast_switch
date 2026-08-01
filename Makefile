# Makefile for STUDY_DPDK_L2_Switch
PKGCONF = pkg-config
CC ?= gcc
CFLAGS += -O3 $(shell $(PKGCONF) --cflags libdpdk)
LDFLAGS += $(shell $(PKGCONF) --libs libdpdk) -lpthread

APP = dpdk_switch
SRCS-y := main.c mac_table.c ipc.c

.PHONY: all clean

all: $(APP)

$(APP): $(SRCS-y)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(APP)
