all:
	g++ -o nat main.c tcpudp.c util.c checksum.c -lnfnetlink -lnetfilter_queue -Wall
