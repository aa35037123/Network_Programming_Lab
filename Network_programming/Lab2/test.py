import socket 
import threading 
from scapy.all import *

MAX_PACKET_SIZE = 2048
HOST = 'localhost'
PORT = 10495
server_addr = (HOST, PORT)
######
#Flag#
######
hello_success = False
chals_success = False

# write to pcap file
PCAP_FILENAME = "./challenge_pkts.pcap"
# listen packet 
filter_condition = f"udp and port {PORT}"
captured_packets = []

def packet_callback(packet):
    print(f'capture packet: {packet}')
    try:
        # pkt_parsed = Ether(str(packet))
        # print(f'parsed packet: {pkt_parsed}')
        # print(f'dst port: {packet[UDP].dport}')
        # print()
        print(f'pkt 2: {packet[2]}')
        if packet.haslayer(UDP) and \
            (packet[UDP].sport == PORT or packet[UDP].dport == PORT):
            captured_packets.append(packet)
    except:
        return

def capture_packets():
    print('Sniffing...')
    capture = sniff(iface="lo", filter="udp and port 10495", prn=lambda packet:packet_callback(packet))

def say_hello(sock, user_id):
    print('Say hello...')
    hello_command = f'hello {user_id}'
    sock.sendto(hello_command.encode(), server_addr)
    time.sleep(0.5)
    capture = sniff(iface="lo", filter="udp and port 10495", prn=lambda packet:packet_callback(packet), timeout=5)
    # capture = sniff(filter="udp and port 10495",timeout=5)

    # chals_respond, addr = sock.recvfrom(MAX_PACKET_SIZE)
    # print('chal respond: {}'.format(chals_respond))
    # capture is a list of packet, each packet has different layer parse by index
    # pkt[2] is udp layer, pkt[2].load get data from udp layer 
    # if use localhost, iface set to 'lo'
    # if use ethernet, iface set to 'eth0'
    return capture

    # (response, address) = sock.recvfrom(MAX_PACKET_SIZE)
    # print(response.decode())
    # chals_id = response.decode().split()[1]
    # return chals_id

def chals(sock, chals_id):
    print('challenge...')
    chals_command = f'chals {chals_id}'
    sock.sendto(chals_command.encode(), server_addr)
    capture = sniff(iface='lo', filter=filter_condition, prn=lambda packet:packet_callback(packet), count=1)
    print(f'length of capture: {len(capture)}')
    return capture

def main():
    # connect to server
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # 创建一个线程来执行捕获数据包的操作
    sock.sendto('hello a'.encode(), server_addr)
    data, addr = sock.recvfrom(MAX_PACKET_SIZE)
    print(addr)
    print(data)
    chal_id = data[3:].decode()
    sock.sendto(f'chals {chal_id}'.encode(), server_addr)
    data, addr = sock.recvfrom(MAX_PACKET_SIZE)
    print(data)
    
    capture_thread = threading.Thread(target=capture_packets)
    capture_thread.start()
    
    global hello_success
    user_id = input('Please input your user_id: ')
    chals_respond = ''
    counter = 0
    while hello_success is not True and counter < 10:
        capture = say_hello(sock, user_id)
        if len(capture) > 0:
            hello_success = True
        counter += 1
    
    print(capture)
    # chals_id = capture[0][2].load.decode()[3:]
    chals_id = chals_respond[3:]
    print(f'chal id: {chals_id}')
    
    # global chals_success
    # while not chals_success:
    #     capture = chals(sock, chals_id)
    #     if len(capture) > 0:
    #         chals_success = True


    # while True:
    #     sock.send(chals_command.encode())
    #     (response, address) = sock.recvfrom(MAX_PACKET_SIZE)
        
    # counter = 0 
    # while counter < 10:
    #     sock.send(chals_command.encode())
    #     (response, address) = sock.recvfrom(MAX_PACKET_SIZE)
    #     # print(f'No decode: {response}')
    #     print(f'After decode: {response.decode()}')
    #     counter += 1    
    #     time.sleep(0.5)

    # Specify the link-layer(ll) type (1 for Ethernet) when writing the pcap file
    wrpcap(PCAP_FILENAME, captured_packets)
    # .join() will block main thread until capture_thread finish
    # capture_thread.join()
    return
if __name__ == '__main__':
    main()