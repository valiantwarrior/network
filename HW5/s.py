# UDPPingClient.py
# 20134220 Jeong hyun, Woo

import socket
import time
import sys

server_ip = "127.0.0.1"
server_port = 24220
server_addr = (server_ip, server_port)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(server_addr)


def main() :
    
    try :
        while True:
            data, client_addr = sock.recvfrom(1024)
            print ("Server: recv \"" + data.decode('utf-8') + "\"")
            time.sleep(1.0)
            sock.sendto(data, client_addr)
            print ("Server: reply \"" + data.decode('utf-8') + "\"")
    
    except KeyboardInterrupt :
        print("Bye~")
        sys.exit(1)

if __name__ == "__main__" :
    main()