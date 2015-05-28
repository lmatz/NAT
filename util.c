#include "util.h"

uint32_t publicIP=0;
uint32_t internalIP=0;
uint32_t subnetIP=0;
unsigned int mask=0xFFFFFFFF;

tcpEntry tcpHead;
udpEntry udpHead;


int processUdp(udpEntry *table, struct ip* packet, double currentTime){
    
    fprintf(stdout,"=============================Process UDP=========================\n");
    int direction = getDirection(packet,publicIP);

    if (direction == 0) {
        // It is an outgoing package, process it
	      fprintf(stdout,"it is an outgoing package\n");
        struct udphdr* udp_hdr = (struct udphdr*) ( (unsigned char*)packet + (packet->ip_hl << 2) );
        
        uint32_t ip_src = ntohl(packet->ip_src.s_addr);
        // ??????? Source port
        uint16_t sport = ntohs(udp_hdr->source);
        udpEntry * entry = findOutUdp(table,ip_src,sport);
        
        uint16_t transPort;
        if ( entry == NULL  ) {
            /* No record yet. Insert Entry */
            
		transPort = insertEntry(table,packet,currentTime);
		fprintf(stdout,"trasport is %u",transPort);	
            if (transPort==-1) {
                // print information for debug.
                    fprintf(stderr,"No avaliable ports\n");
                // so we need to indicate the Callback function in main.c to drop the packet.
                return 0;
            }
        }else if(currentTime - entry->createTime > 30){
            /* Record found, but has expired. */
            removeUdp(entry);
            transPort = insertEntry(table,packet,currentTime);
            
            //print information for debug
            

            if (transPort==-1) {
                // print information for debug.
//                if (debug==1) {
//                    fprintf(stderr,"No avaliable ports\n");
//                }
                // so we need to indicate the Callback function in main.c to drop the packet.
                return 0;
            }
        }else{
            /* Record found and valid. Reset the clock! */
            entry->createTime = currentTime;
            transPort = entry->transPort;
            //print information for debug
            printUdp(entry);
        }
        

        /*
         Change the source IP address to that of VM A and the source port number to transPort
         */
        packet->ip_src.s_addr = htonl(publicIP);
	
	printf("s_addr: %u !!!\n",ntohl(packet->ip_src.s_addr));
	
        //set the source port
        udp_hdr->source = htons(transPort);
	
	packet->ip_sum = ip_checksum((unsigned char *)packet);	
        
	// Update the checksum
        udp_hdr->check= udp_checksum((unsigned char *)packet);
        
	
	// packet->ip_sum = htons (ip_checksum((unsigned char *)packet));
        
	show_checksum((unsigned char*)packet,0,transPort);


	fprintf(stdout,"quit the processUdp!!!\n");
	// return 1 to let Callback function know we need to accept the packet.
        return 1;
        
    }else{
        // It is an incoming package, process it
	
	fprintf(stdout,"it is an incoming package\n");
        struct udphdr* udp_hdr = (struct udphdr*) ( (unsigned char*)packet + (packet->ip_hl << 2) );
        
        uint16_t dport = ntohs(udp_hdr->dest);
      //  uint32_t ip_dest = ntohs(packet->ip_dst.s_addr);
        //?????????? Where is the destination port
        udpEntry *entry = findInUdp(table,dport);


        if (entry == NULL || currentTime - entry->createTime > 30) {
            // return 0 to let Callback function know we need to drop the packet.
            return 0;
        }else{
            // Record found, and valid, reset the clock
            entry -> createTime = currentTime;
            packet->ip_dst.s_addr = htonl(entry->ip);
            // ??????????????? Where is the port field
            udp_hdr->dest = htons(entry->port);
	    udp_hdr->check = udp_checksum((unsigned char *)packet);
            packet->ip_sum = ip_checksum((unsigned char *)packet);
        }
        return 1;
    }
}

int processTcp(tcpEntry *table, struct ip* packet) {
	fprintf(stdout,"=============================Process TCP=============================\n");
	int direction = getDirection(packet, publicIP);
	

    struct tcphdr* tcp_hdr2 = (struct tcphdr*) ( (unsigned char*) packet + (packet->ip_hl << 2) );
	if (tcp_hdr2->ack==1) {
		fprintf(stdout,"IT is an ACK packet!!!\n");
	}
	if (tcp_hdr2->fin==1) {
		fprintf(stdout,"IT is an FIN acket!!!\n");
	}
	if (direction == 0) {
		//Outbound
		fprintf(stdout, "outgoing TCP package\n");
    struct tcphdr* tcp_hdr = (struct tcphdr*) ( (unsigned char*) packet + (packet->ip_hl << 2) );

    uint32_t ip_src = ntohl(packet->ip_src.s_addr);
    uint16_t sport = ntohs(tcp_hdr->source);
    tcpEntry * entry = findOutTcp(table, ip_src, sport);

	

    uint16_t transPort;
    if ( entry == NULL ) {
      // No record yet.
      if (tcp_hdr->syn == 1) {
        fprintf(stdout,"This is a SYN packet. Entry NULL!\n");
        // create a new entry in the translation table
        transPort = insertTcpEntry(table, packet);
      } else {
	fprintf(stdout,"This is not a SYN packet. Entry NULL!\n");
        return 0;
      }
    } else {

      fprintf(stdout,"Non-SYN. Entry found with transport: %d. Information: \n",entry->transPort);
      // There is a record.
      // print information for debug
      printTcp(entry);
      //if (tcp_hdr->syn != 1) {
        // use previously assigned port number
        transPort = entry->transPort;
	fprintf(stdout,"Transport changed to %d\n",transPort);
      //}
    }

    fprintf(stdout,"Transport: %d\n",transPort);

    packet->ip_src.s_addr = htonl(publicIP);

    tcp_hdr->source = htons(transPort);

    packet->ip_sum = ip_checksum((unsigned char *)packet);
    tcp_hdr->check = tcp_checksum((unsigned char *)packet);

    //packet->ip_sum = htons(ip_checksum((unsigned char *)packet));

    return 1;
  }else if (direction == 1) {

	fprintf(stdout,"Incoming packet!!!\n");
    //Inbound
    struct tcphdr* tcp_hdr = (struct tcphdr*) ((unsigned char*) packet + (packet->ip_hl << 2));

    uint16_t dport = ntohs(tcp_hdr->dest);
    //uint32_t ip_dest = ntohs(packet->ip_dst.s_addr);
    tcpEntry *entry = findInTcp(table, dport);

    if (entry == NULL) {
      return 0;
    } else {
      packet->ip_dst.s_addr = htonl(entry->ip);
      tcp_hdr->dest = htons(entry->port);
      tcp_hdr->check = tcp_checksum((unsigned char*)packet);
      packet->ip_sum = ip_checksum((unsigned char*)packet);

    }
    return 1;

	}
  return 1;
}

int getDirection(struct ip* packet, uint32_t Aip){
    uint32_t desip = ntohl(packet->ip_dst.s_addr); 
    
    if ( desip  == Aip) {
        // If the destination ip is the same as the ip of VM A, then it is an incoming package.
        return 1;
    }
    else{
        // Otherwise, it is an outgoing package.
        return 0;
    }
}

int insertEntry(udpEntry *table,struct ip* packet,double currentTime){
    int transPort = getAvailableTransport();
    struct udphdr* udp_hdr = (struct udphdr*) ( (unsigned char*)packet + (packet->ip_hl << 2) );
    if (transPort > 0) {
        uint32_t srcip = ntohl(packet->ip_src.s_addr);
        // port?
        uint16_t port = ntohs(udp_hdr->source);
        addUdp(table,srcip,port,transPort,currentTime);
        return transPort;
    }else{
        /* Maybe add some error handling mechanism. */
        // return -1, we will drop this packet cause the port number is full.
        return transPort;
    }
}


int getAvailableTransport(){
    uint16_t port = 10000;
    int i;
    for (i=0; i<=2000; i++) {
        port = 10000 + i;
        udpEntry * res = findInUdp(&udpHead,port);
        if (res) {
          printf("udp port : %u",port);
        }
        tcpEntry * res2= findInTcp(&tcpHead,port);
        if (res2) {
          printf("tcp port : %u",port);
        }
        if (!res && !res2) {
            return port;
        }
    }
    return -1;
}

int insertTcpEntry(tcpEntry *table, struct ip* packet) {
  int transPort = getAvailableTransport();
  struct tcphdr* tcp_hdr = (struct tcphdr*) ( (unsigned char*)packet + (packet->ip_hl << 2));
  if (transPort > 0) {
    uint32_t srcip = ntohl(packet->ip_src.s_addr);
    uint16_t port = ntohs(tcp_hdr->source);
    addTcp(table,srcip,port,transPort);
    return transPort;
  } else {
    return transPort;
  }
}
