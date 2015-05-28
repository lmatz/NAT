VMID="40"		# group no.
IP="10.3.1.${VMID}"	# public interface
LAN="10.0.${VMID}.0"	# private LAN network address (without subnet mask)
MASK="27"

echo "1" >  /proc/sys/net/ipv4/ip_forward
iptables -t nat -F
iptables -t filter -F
iptables -t mangle -F
iptables -t filter -A FORWARD -j NFQUEUE --queue-num 0 -p tcp -s ${LAN}/${MASK} ! -d ${IP} --dport 10000:12000
iptables -t mangle -A PREROUTING -j NFQUEUE --queue-num 0 -p tcp -d ${IP} --dport 10000:12000
iptables -t filter -A FORWARD -j NFQUEUE --queue-num 0 -p udp -s ${LAN}/${MASK} ! -d ${IP} --dport 0:12000

iptables -t mangle -A PREROUTING -j NFQUEUE --queue-num 0 -p udp -d ${IP} --dport 0:12000
#iptables -A OUTPUT -p icmp -j NFQUEUE --queue-num 0
#iptables -A INPUT -p udp -j NFQUEUE --queue-num 0 -s ${LAN}/${MASK} 
