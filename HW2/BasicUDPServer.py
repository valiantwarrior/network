#
# 20134220 Jeong hyun, Woo
# BasicUDPServer.py
#


from socket import *
from datetime import datetime
import sys

serviceCount = 0

serverPort = 24220
serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind(('', serverPort))

print("The server is ready to receive on port", serverPort)

try:
    while True :
        # receive command from client
        option, clientAddress = serverSocket.recvfrom(2048)
        print('Connection requested from',clientAddress)
        print('Command',option.decode())
        
        if int(option.decode()) == 1:
            # receive sentence from client
            message, clientAddress = serverSocket.recvfrom(2048)
            modifiedMessage = message.decode().upper()
            # send uppercase-sentece to client
            serverSocket.sendto(modifiedMessage.encode(),clientAddress)
            serviceCount += 1

        elif int(option.decode()) == 2:
            # send client's ip & port to client
            serverSocket.sendto((str(clientAddress[0])+','+str(clientAddress[1])).encode(),clientAddress)
            serviceCount += 1
            
        elif int(option.decode()) == 3:
            # send server's current time to client 
            message = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            serverSocket.sendto(str(message).encode(),clientAddress)
            serviceCount += 1

        elif int(option.decode()) == 4:
            # send server's service count to clinet 
            serviceCount += 1
            serverSocket.sendto(str(serviceCount).encode(),clientAddress)

        else :
            continue

# when user terminate program by ctrl + c
except KeyboardInterrupt:
    serverSocket.close()
    print('\nBye bye~~')