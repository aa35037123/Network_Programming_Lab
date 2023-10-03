from scapy.all import *

if __name__ == '__main__':
    pcap_file = 'test.pcap'
    captured_packets = rdpcap(pcap_file)
    print(f'captured_packets: {captured_packets}')
    name = 'Wesley'
    print(name[:3])