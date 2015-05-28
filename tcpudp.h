#ifndef __UDP_H

#define __UDP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "list.h"


/*              UDP LIST            */

typedef struct {
	uint32_t ip;
	uint16_t port;
	uint16_t transPort;
	double createTime;

	struct list_head link;
} udpEntry;


void initUdp( udpEntry *head );
void addUdp( udpEntry *head, uint32_t ip, uint16_t port, uint16_t transPort, double createTime);
udpEntry* findInUdp( udpEntry *head, uint16_t transPort);
udpEntry* findOutUdp( udpEntry *head, uint32_t ip, uint16_t port);
void removeUdp(udpEntry *node);
void removeUdpHead( udpEntry *head);
void printUdp( udpEntry *node);
void printAllUdp(udpEntry *head);


/*              TCP LIST            */

typedef struct {
	uint32_t ip;
	uint16_t port;
	uint16_t transPort;

	struct list_head link;
} tcpEntry;


void initTcp( tcpEntry *head );
void addTcp( tcpEntry *head, uint32_t ip, uint16_t port, uint16_t transPort);
tcpEntry* findInTcp( tcpEntry *head, uint16_t transPort);
tcpEntry* findOutTcp( tcpEntry *head, uint32_t ip, uint16_t port);
void removeTcp(tcpEntry *node);
void removeTcpHead( tcpEntry *head);
void printTcp( tcpEntry *node);
void printAllTcp( tcpEntry *head);




#endif
