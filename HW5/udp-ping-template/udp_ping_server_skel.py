# server_skel.py
import socket
import time

server_ip = "127.0.0.1"
server_port = 9000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((server_ip, server_port))
	
while True:
    data, addr = sock.recvfrom(1024)
    print ("Server: recv \"" + data.decode('utf-8') + "\"")
    
    sock.sendto(data, addr)
    print ("Server: reply \"" + data.decode('utf-8') + "\"")
