# client_skel.py
import socket

server_ip = "127.0.0.1"
server_port = 9000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

message = "Ping"

sock.sendto(message.encode(), (server_ip, server_port))
print ("Client: send \"" + message + "\"")

data, addr = sock.recvfrom(1024)
print ("Client: recv \"" + data.decode('utf-8') + "\"")
