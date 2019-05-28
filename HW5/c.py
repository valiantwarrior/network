# UDPPingClient.py
# 20134220 Jeong hyun, Woo

import socket
import queue
import sys
import getopt
import time

server_ip = "127.0.0.1"
server_port = 24220
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(0.9)
packet_queue = queue.Queue()
receive_queue = queue.Queue()

# command line argument is invalid, print guide for user
def help_for_cmd() :
    print("<OPTION LIST & USAGE>")
    print("-c < IP address  > : Set IP address of the UDP ping server. If not specified, default is 127.0.0.1.")
    print("-p < Port number > : Set Port number of the UDP ping server. If not specified, default is 24220.")
    print("-w < timeout(ms) > : Set the timeout time. If not specified, default is 1000ms.")

def print_result() :
    pass

# define command line argument
def setup_with_command() :
    global server_ip, server_port, sock
    try :
        option_list, args = getopt.getopt(sys.argv[1:],"c:p:w:")
        if option_list :
            for option in option_list :
                if '-c' in option :
                    server_ip = option[1]
                    continue
                if '-p' in option :
                    server_port = int(option[1])
                    continue
                if '-w' in option :
                    timeout = float(option[1]) / 1000
                    sock.timeout(timeout)
                    continue             
    except getopt.GetoptError : 
        help_for_cmd()
        sys.exit(1)


# initialize packet_queue
def init_packet() :
    for i in range(0,10) :
        packet_queue.put_nowait(i)

# send
def send_to_server(_packet) :
    sock.sendto(str(_packet).encode(), (server_ip, server_port))
    print("PING<{0}> sent".format(_packet))

    

# receive
def receive_from_server(_packet) :
    try :
        data, addr = sock.recvfrom(1024)
        data = int(data.encode())
        receive_queue.put_nowait(data)
        if data is not _packet :
            return False

        print("PING<{0}> reply received from <{1}> : RTT = <{2}>ms".format(data, addr[0], 10))
        return True
    
    except socket.timeout : 
        print("PING<{0}> timeout!".format(_packet))
        return False

# main
def main() :

    try :
        
        setup_with_command()
        init_packet()
        
        while True :
            packet = packet_queue.get_nowait()
            send_to_server(packet)
            receive_from_server(packet)
            if packet_queue.qsize() == 0 : 
                break
        print_result()
        
        sock.close()
    except KeyboardInterrupt :
        sock.close()
        print("\nBye~")
        sys.exit(1)

if __name__ == "__main__" :
    main()