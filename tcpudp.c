#include "tcpudp.h"





/*             UDP LIST             */

/* intialize the list head. head's next pointer and prev pointer both point to this head.
	this head is really just a dummy head. All new entry of list will
	be added to the next of the dummy head.
*/
void initUdp( udpEntry *head) {
	INIT_LIST_HEAD(&(head->link));
}


/* make a new udp entry, and add it to the first of the list 
   just as said above, "first" means the first entry of the list except the dummy head
*/
void addUdp(  udpEntry *head, uint32_t ip, uint16_t port, uint16_t transPort, double createTime) {
	udpEntry *newEntry = (udpEntry *)malloc(sizeof(udpEntry));

	newEntry->ip = ip;
	newEntry->port = port;
	newEntry->transPort = transPort;
	newEntry->createTime = createTime;

	list_add(&(newEntry->link), &(head->link));
}


/* look for inbound record
	you may need this when you are trying to transport packet from external networks to internal networks.
*/
udpEntry* findInUdp( udpEntry* head, uint16_t transPort) {

	udpEntry *pos;

	list_for_each_entry(pos, &(head->link), link) {
		if (pos->transPort == transPort) {
			return pos;
		}
	}
	return NULL;

}

/* look for outbound record
	you may need this when you are trying to transport packet from internal networks to external networks.
*/
udpEntry* findOutUdp(udpEntry *head, uint32_t ip, uint16_t port) {

	udpEntry *pos;

	list_for_each_entry(pos, &(head->link), link) {
		if (pos->ip == ip && pos->port == port) {
			return pos;
		}
	}

	return NULL;
}

/* delete the udp entry.  ex: if this udp entry excess the time allowed */
void removeUdp(udpEntry *node) {
	list_del(&(node->link));
	free(node);
}

/* delete all the udp entry. 
*/
void removeUdpHead(udpEntry *head) {

	udpEntry *pos, *store;

	list_for_each_entry_safe(pos, store, &(head->link), link) {
		removeUdp(pos);
	}
}


/* print the udp entry. 
*/
void printUdp(udpEntry *node) {
	printf("[%x, %u, %u, %lf]\n", node->ip, node->port, node->transPort, node->createTime);
}


/* print all the udp entry
*/
void printAllUdp(udpEntry *head) {

	udpEntry *pos;

	list_for_each_entry(pos, &(head->link), link) {
		printUdp(pos);
	}

}


/*             TCP LIST             */

void initTcp( tcpEntry *head ) {
	INIT_LIST_HEAD(&(head->link));
}

void addTcp( tcpEntry *head, uint32_t ip, uint16_t port, uint16_t transPort) {
	tcpEntry *newEntry = (tcpEntry *)malloc(sizeof(tcpEntry));

	newEntry->ip = ip;
	newEntry->port = port;
	newEntry->transPort = transPort;

	list_add(&(newEntry->link), &(head->link));
}

tcpEntry* findInTcp( tcpEntry *head, uint16_t transPort) {

	tcpEntry *pos;

	list_for_each_entry(pos, &(head->link), link) {
		if (pos->transPort == transPort) {
			return pos;
		}
	}

	return NULL;
}


tcpEntry* findOutTcp( tcpEntry *head, uint32_t ip, uint16_t port) {

	tcpEntry *pos;

	list_for_each_entry(pos, &(head->link), link) {
		if (pos->ip == ip && pos->port == port) {
			return pos;
		}
	}

	return NULL;
}

void removeTcp(tcpEntry *node) {
	list_del(&(node->link));
	free(node);
}

void removeTcpHead(tcpEntry *head) {

	tcpEntry *pos, *store;

	list_for_each_entry_safe(pos, store, &(head->link), link) {
		removeTcp(pos);
	}
}

void printTcp(tcpEntry *node) {
	printf("[%x, %u, %u]\n", node->ip, node->port, node->transPort);
}

void printAllTcp(tcpEntry *head) {

	tcpEntry *pos;

	list_for_each_entry(pos, &(head->link), link) {
		printTcp(pos);
	}

}

