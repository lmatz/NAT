#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>				/* SIGINT */
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>			/* "netfilter.h" */
extern "C" {
	#include <linux/netfilter.h>	/* NF_ACCEPT, NF_DROP */
	#include <libnetfilter_queue/libnetfilter_queue.h>
}
#include <arpa/inet.h>			/* ntoh[s|l]() */
#include <netinet/ip.h>			/* "struct iph" */
#include <netinet/tcp.h>		/* "struct tcph" */
#include <netinet/udp.h>		/* "struct udph" */
#include <sys/types.h>			/* required by "inet_ntop()" */
#include <sys/socket.h>			/* required by "inet_ntop()" */
#include <arpa/inet.h>			/* required by "inet_ntop()" */

#include "list.h"
#include "tcpudp.h"
#include "checksum.h"




#define MIN_PORT 10000	// the port used by VM A for translation should be >= 10000 and <= 12000 
#define MAX_PORT 12000
#define UDP_TIME 30     // a udp entry can only last 30 seconds if it hasn't beed used in this 30 seconds.


extern uint32_t publicIP;    // the ip address used by VM A to communicate with external networks
extern uint32_t internalIP;  // the internal network ip address
extern uint32_t subnetIP;	  // the subnet network ip address
//extern int debug;
//tcpEntry tcpHead;
//udpEntry udpHead;
extern unsigned int mask;


int  processTcp(tcpEntry *table, struct ip* packet);
int  processUdp(udpEntry *table, struct ip* packet, double currentTime);
int getDirection(struct ip* packet, uint32_t Aip);
int insertEntry(udpEntry *table, struct ip* packet, double currentTime);
int insertTcpEntry(tcpEntry *table, struct ip* packet);
int getAvailableTransport();


#endif
