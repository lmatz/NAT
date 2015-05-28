#include "util.h"

#define BUFF_SIZE 4096

extern uint32_t publicIP;    // the ip address used by VM A to communicate with external networks
extern uint32_t internalIP;  // the internal network ip address
extern uint32_t subnetIP;	  // the subnet network ip address
extern unsigned int mask;
extern tcpEntry tcpHead;
extern udpEntry udpHead;

//extern int debug;



static int Callback123(nfq_q_handle* myQueue, struct nfgenmsg* msg, nfq_data* pkt, void *cbData);




int main(int argc, char **argv) {

	struct nfq_handle *nfqHandle;
	struct nfq_q_handle *myQueue;
	struct nfnl_handle *netlinkHandle;

	int fd, res;
	char buf[BUFF_SIZE];

	if (argc!=4) {
		fprintf(stderr,"Error in number of arguments\n");
		exit(-1);
	}

	// get the public IP address. And the public IP address is host byte order.
	publicIP = inet_network(argv[1]);
	if (publicIP == 0) {
		fprintf(stderr,"Error in public IP\n");
		exit(-1);
	}
	fprintf(stdout,"publicIP: %u\n",publicIP);

	// get the subnet IP address. And the subnet IP address is host byte order.
	internalIP = inet_network(argv[2]);
	if (internalIP == 0) {
		fprintf(stderr,"Error in internal IP\n");
		exit(-1);
	}
	fprintf(stdout,"internalIP: %u\n",internalIP);
	mask = mask << (32-atoi(argv[3]));
	subnetIP = internalIP & mask;
	fprintf(stdout,"subnetIP: %u\n",subnetIP);

	initUdp( &udpHead );
	initTcp( &tcpHead );

	if ( !(nfqHandle = nfq_open())) {
		fprintf(stderr, "Error in nfq_open()\n");
		exit(-1);
	}

	if ( nfq_unbind_pf(nfqHandle, AF_INET) < 0 ) {
		fprintf(stderr, "Error in nfq_unbind_pf()\n");
		exit(-1);
	}

	if ( nfq_bind_pf(nfqHandle, AF_INET) < 0) {
		fprintf(stderr, "Error in nfq_bind_pf()\n");
		exit(-1);
	}

	if ( !(myQueue = nfq_create_queue(nfqHandle, 0, &Callback123, NULL)) ) {
		fprintf(stderr, "Error in nfq_create_queue()\n");
		exit(1);
	}

	if ( nfq_set_mode(myQueue, NFQNL_COPY_PACKET, 0xffff) <0 ) {
		fprintf(stderr, "Could not set packet copy mode\n");
		exit(1);
	}

	netlinkHandle = nfq_nfnlh(nfqHandle);
	fd = nfnl_fd(netlinkHandle);

	
	fprintf(stdout,"ready to receive packets\n");

	//Start to process the packet we receive.
	while ( (res = recv(fd, buf , sizeof(buf), 0))  && res>=0 ) {
		
		printf("\n\n\n*******NEW ONE*******\n");
		fprintf(stdout,"in the loop\n");
		nfq_handle_packet(nfqHandle, buf , res);
	}
	//End the process


	nfq_destroy_queue(myQueue);

	nfq_close(nfqHandle);

	return 0;

}




static int Callback123(nfq_q_handle* myQueue, struct nfgenmsg* msg, nfq_data* pkt, void *cbData) {
	unsigned int id=0;
	nfqnl_msg_packet_hdr *header;
	double currentTime=0;
	
	fprintf(stdout,"enter the callback\n");	
  
  fprintf(stdout,"***********ALL entry in UDP table\n");
  printAllUdp(&udpHead);
  fprintf(stdout,"\n**********All entry in tCP table\n");
  printAllTcp(&tcpHead);
  fprintf(stdout,"\n*****end***************\n\n");


	//get time
//	struct timeval tv;
//	if (!nfq_get_timestamp(pkt,&tv) ) {
//		double currentTime = tv.tv_sec + (double) tv.tv_usec / 1000000;
//		printf(" Timestamp: %lf\n",currentTime);
//	}
//	else {
//		printf(" Timestamp: nil\n");
//	}
	
  time_t now;
  now=time(0);
  currentTime=(double)now;


		printf("pkt recvd: ");
		if ( (header = nfq_get_msg_packet_hdr(pkt) )) {
			id = ntohl(header->packet_id);
			printf("  id: %u\n", id);
			printf("  hw_protocol: %u\n", ntohs(header->hw_protocol));		
			printf("  hook: %u\n", header->hook);
		}
		

	unsigned char *pktData;
	int len=nfq_get_payload(pkt, (char**)&pktData);

	/*
		convert the pktData to ip header and check the ip header's protocol:
		TCP :  process
		UDP :  process
		Neither TCP or UDP: drop
	*/
	struct ip* ip_hdr = (struct ip *) pktData;
	switch(ip_hdr->ip_p) {
		case IPPROTO_TCP:
		{
//			struct tcphdr* tcp_hdr = (struct tcphdr*) ( (unsigned char*)pktData + (ip_hdr->ip_hl << 2) );
			
			int processResult = processTcp(&tcpHead,ip_hdr);
			
			if ( processResult==1 ) {
				return nfq_set_verdict(myQueue, id, NF_ACCEPT, len, pktData);
			}
			else if ( processResult==0 ) {
				return nfq_set_verdict(myQueue, id, NF_DROP, 0, NULL);
			}
			else {
				fprintf(stderr,"The processUdp gives a wrong number, don't know whether to drop or accept\n");
				return nfq_set_verdict(myQueue, id, NF_DROP, 0, NULL);
			}
			break;
		}
		
		case IPPROTO_UDP:
		{
		//	struct udphdr* udp_hdr = (struct udphdr*) ( (unsigned char*)pktData + (ip_hdr->ip_hl << 2) );
			/*
				UDP part
			*/

			int processResult = processUdp(&udpHead,ip_hdr,currentTime);
			if ( processResult==1 ) {
				printf("Packet Source: %u\n",ntohl(ip_hdr->ip_src.s_addr));
				fprintf(stdout,"Packet Accepted. \n");
				return nfq_set_verdict(myQueue, id, NF_ACCEPT, len, pktData);
			}
			else if ( processResult ==0 ) {
				fprintf(stdout,"Packet Dropped. \n");
				return nfq_set_verdict(myQueue, id, NF_DROP, 0, NULL);
			}
			else {
				fprintf(stderr,"The processUdp gives a wrong number, don't know whether to drop or accept\n");
				return nfq_set_verdict(myQueue, id, NF_DROP, 0, NULL);
			}
			break;
		}
		default:
		{
			printf("Neither TCP or UDP, just drop it");
			return 	nfq_set_verdict(myQueue, id, NF_DROP, 0, NULL);
			break;
		}
	}

}
