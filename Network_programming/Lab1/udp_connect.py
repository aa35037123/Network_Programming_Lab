import socket 
import threading 
import os
import subprocess
import signal
import numpy as np
from scapy.all import *

MAX_PACKET_SIZE = 1024
HOST = 'localhost'
PORT = 10495
server_addr = (HOST, PORT)
######
#Flag#
######
hello_success = False
chals_success = False
begin_flag = False
# begin_seq = None
end_flag = False
# end_seq = None

packet_dict = {}

def say_hello(sock, user_id):
    print('Say hello...')
    hello_command = f'hello {user_id}'
    sock.sendto(hello_command.encode(), server_addr)
    respond, addr = sock.recvfrom(MAX_PACKET_SIZE)
    print(respond)
    chals_id = respond.decode()[3:]
    # capture is a list of packet, each packet has different layer parse by index
    # pkt[2] is udp layer, pkt[2].load get data from udp layer 
    # if use localhost, iface set to 'lo'
    # if use ethernet, iface set to 'eth0'
    return chals_id

def chals(sock, chals_id):
    print('challenge...')
    chals_command = f'chals {chals_id}'
    sock.sendto(chals_command.encode(), server_addr)
    # data, addr = sock.recvfrom(MAX_PACKET_SIZE)
    
def parsing_pcap(pcap_file):
    captured_packets = rdpcap(pcap_file)
    print(f'captured_packets: {captured_packets}')
    sessions = captured_packets.sessions()
    udp_packets = sessions['Other'][UDP]    
    len_pkts = len(udp_packets)
    print(f'udp packet len: {len_pkts}')
    print(udp_packets)
    begin_seq = None
    end_seq = None
    # value of seq_array is the ascii code 
    # which represent length of ip option and udp payload
    seq_array = [0] * 300
    for i in range(len_pkts):
        # show() can show the layer that in
        udp_packets[i].show()
        udp_payload = udp_packets[i][Raw].load
        try:
            data = udp_payload.decode()
        except Exception as e:
            print(f"Error processing packet: {e}")
        print(data)
        # IHL means the length of ip header
        ip_header_len = udp_packets[i][IP].ihl * 4
        ip_options_len = ip_header_len - 20
        # udp header length is 8 byte
        udp_len = udp_packets[i][UDP].len
        udp_payload_len = udp_len - 8
        print(f'ip_option len : {ip_options_len}')
        print(f'udp_payload len : {udp_payload_len}')
        x_header_len = ip_options_len + udp_payload_len
        if data[:3] == 'SEQ':
            seq_num = int(data[4:9])
            print(type(seq_num))
            print(f'seq_num: {seq_num}')
            flag = data[10:]
            print(f'flag: {flag}')
            if flag == 'BEGIN FLAG\n':
                begin_seq = seq_num
                print(f'find begin flag, begin seq: {begin_seq}')
            elif flag == 'END FLAG\n':
                end_seq = seq_num
                print(f'find end flag, end seq: {end_seq}')
            else:
                print('Doesn\'t find flag')
            seq_array[seq_num] = chr(x_header_len)
    print(f'begin seq: {begin_seq}, end_seq: {end_seq}')
    print(f'Seq array: {seq_array}')
    flag = ''
    for char in seq_array[begin_seq+1:end_seq]:
        flag += char
    print(f'Flag: {flag}')
    return flag

def verify(sock, flag):
    print('verfy...')
    verify_command = f'verfy {flag}'
    sock.sendto(verify_command.encode(), server_addr)
    # time.sleep(1)
    # respond, addr = sock.recvfrom(MAX_PACKET_SIZE)
    # print(respond.decode())
    return

def main():
    # connect to server
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sniff = "tcpdump -ni any -Xxnv udp and port 10495 and host 127.0.0.1 -w test.pcap -s 1000"
    process_sniff = subprocess.Popen(sniff, shell=True)
    # capture_thread = threading.Thread(target=capturing)
    # capture_thread.start()
    
    global hello_success
    user_id = input('Please input your user_id: ')
    counter = 0
    while hello_success is not True and counter < 10:
        chals_id = say_hello(sock, user_id)
        if len(chals_id) > 0 and chals_id != '\n':
            hello_success = True
        counter += 1

    chals(sock, chals_id)
    # 等待命令执行完成
    try:
        outs, errs = process_sniff.communicate(timeout=15)
    except subprocess.TimeoutExpired:
        process_sniff.kill()
        outs, errs = process_sniff.communicate()
    # os.killpg(os.getpgid(process_sniff.pid), signal.SIGTERM)
    pcap_file = "./test.pcap"
    # success = False
    # while not success:
    #     parsing_pcap()
    flag = parsing_pcap(pcap_file)
    verify(sock, flag)
    return  
if __name__ == '__main__':
    main()