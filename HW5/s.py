# UDPPingClient.py
# 20134220 Jeong hyun, Woo

import socket
import time
import sys
import random

server_ip = "127.0.0.1"
server_port = 24220
server_addr = (server_ip, server_port)
client_list = []

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(server_addr)

def processing_simulator() :
    delay = random.random() * 2.0
    time.sleep(delay)
    print(delay)

def packet_loss_simulator() :
    return True if random.random() >= 0.8 else False


def ping_simulator(data, client_addr) :
    
    print(client_addr)
    print("Server: recv \"" + data.decode('utf-8') + "\"")
    
    if packet_loss_simulator() is False :
        processing_simulator()
        sock.sendto(data, client_addr)
        print ("Server: reply \"" + data.decode('utf-8') + "\"")
    
    else :
        print ("Ping{0} dropped".format(data.decode()))

def multi_client_handler(client_addr) :
    pass

def main() :
    
    try :
        while True:
            data, client_addr = sock.recvfrom(64)
            for client_info in client_list :
                if client_addr in client_info :
                    pass
                else :
                    pass                
            ping_simulator(data,client_addr)
    
    except KeyboardInterrupt :
        print("Bye~")
        sys.exit(1)

if __name__ == "__main__" :
    main()