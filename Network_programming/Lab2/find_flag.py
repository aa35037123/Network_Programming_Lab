# import requests as rq
import os
import struct
import requests
from bs4 import BeautifulSoup

def bxor(b1, b2):
    result = bytearray(b1)
    for i, b in enumerate(b2):
        result[i] ^= b
    return bytes(result)

def main():
    # challenge_url = 'https://inp.zoolab.org/binflag/challenge?id=110611008'
    # demo_filename = 'challenge.bin'
    
    # os.system(f"curl {challenge_url} --output {demo_filename}")

    # with open(f"{challenge_filename}", "rb") as f:
    #     data = f.read()
    # print(type(data))
    # print(data)
    # print(f'data length: {len(data)}')

    demo_filename = 'demo1.bin'

    # deal with header
    block_dict = {}
    with open(f"{demo_filename}", "rb") as f:
        # args of f.read() is bytes you want to read in
        header_byte = f.read(16)
        magic, datasize, n_blocks, zeros, = struct.unpack('>8sIHH', header_byte)
        print(f'datasize: {datasize}')
        print(f'block numbers: {n_blocks}')
        # print(f"magic: {magic}, datasize: {datasize}, n_blocks: {n_blocks}, zeros: {zeros}")
        # print(demo_byte)
        # (magic,) = struct.unpack('>8s', demo_byte)
        # print(f"magic: {magic}")

        # deal with each block
        for i in range(n_blocks):
            block_byte = f.read(8)
            offset, cksum, length = struct.unpack('>IHH', block_byte)
            print(f'offset: {offset}, cksum: {cksum}, length: {length}')   
            payload = f.read(length)
            print(f'payload: {payload}')
            result = bxor(payload[0:2], payload[2:4])
            i = 4
            # checksum
            while(i < length):
                result = bxor(result, payload[i:i+2])
                i += 2

            # for i in range(1, length//2 ):
            #     next_2byte = payload[i*2:(i+1)*2]
            #     result = bxor(result, next_2byte)
            
            result = int.from_bytes(result)
            print(f'result cksum: {result}')
            if (result == cksum):
                block_dict[offset] = payload
        
        # print(f'before sort: {block_dict}\n')
        sorted_dict = dict(sorted(block_dict.items()))
        # print(f'after sort: {sorted_dict}\n')
            # for 
            # print(payload)
        # print(f'block dict: {sorted_dict}')
        flag_length = f.read(2)
        length, = struct.unpack('>H', flag_length)
        print(f'length: {length}')
        byte_str = b''
        # use offset to rearrange every block
        for item in sorted_dict:
            byte_str += sorted_dict[item]
            # print(sorted_dict[item])
        answer = b''
        # print(f'byte str: {byte_str}')
        # deal with flag, use flag find the answer hidden in all blocks
        flags = []
        for i in range(length):
            raw_flag = f.read(4)
            flag, = struct.unpack('>I', raw_flag)
            # print(f'current add: {byte_str[flag:flag+2]}')
            answer += byte_str[flag:flag+2]
            flags.append(flag)  
        # print(f'flags: {flags}')
        # print(f'flag: {flags}')
        # print(f'byte answer flag: {answer}')
        print(f'hex answer flag: {answer.hex()}')

        ## get response from server
        final_request = f'https://inp.zoolab.org/binflag/verify?v={answer.hex()}'
        final_filename = 'output.txt'

        try:
            response = requests.get(final_request)
            print(response.text)
            # if response.status_code == 200:
            #     soup = BeautifulSoup(response.text, 'html.parser')
            #     all_text = ' '.join(soup.stripped_strings)
            #     print(all_text)
            # else:
            #     print(f"Request failed with status code: {response.status_code}")
        except requests.RequestException as e:
            print("An error occurred:", e)

        # os.system(f"curl {final_request} --output {challenge_filename}")
        # with open(final_filename, 'r') as f:
        #     for line in f:
        #         print(line, end='')
if __name__ == '__main__':
    main()  