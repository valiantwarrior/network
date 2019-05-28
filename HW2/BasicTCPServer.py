#
# 20134220 Jeong hyun, Woo
# BasicTCPServer.py
#


from socket import *
from datetime import datetime
import sys

serviceCount = 0

serverPort = 24220
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('', serverPort))
serverSocket.listen(1)


try :

    print("The server is ready to receive on port", serverPort)
    (connectionSocket, clientAddress) = serverSocket.accept()
    
    while True:

        print('Connection requested from', clientAddress)

        # receive command from client
        option = connectionSocket.recv(2048)
        
        # If client exit the program by 'Ctrl + C' or 'Ctrl + Z', wait for new client
        if option.decode() == '' :
            print('Command 5')
            connectionSocket.close()
            (connectionSocket, clientAddress) = serverSocket.accept()
            continue
        

        print('Command', option.decode())

        # option 5 : when client choose 'exit', server need to make new accept
        if int(option.decode()) == 5:
            connectionSocket.close()
            (connectionSocket, clientAddress) = serverSocket.accept() 


        # option 1 : simple echo.
        if int(option.decode()) == 1:                                   
            message = connectionSocket.recv(2048)
            
            # Client exit program by 'Ctrl + c' or 'Ctrl + z' after choosing option 1, wait for new client.
            if message.decode() == '':
                connectionSocket.close()
                (connectionSocket, clientAddress) = serverSocket.accept()
                continue
            
            modifiedMessage = message.decode().upper()
            connectionSocket.send(modifiedMessage.encode())
            serviceCount += 1

        # option 2 : send client's IP & port info.
        elif int(option.decode()) == 2:                                 
            connectionSocket.send((clientAddress[0]+','+str(clientAddress[1])).encode())
            serviceCount += 1

        # option 3 : send server's current time info.
        elif int(option.decode()) == 3:                                 
            message = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            connectionSocket.send(str(message).encode())
            serviceCount += 1

        # option 4 : send service count.
        elif int(option.decode()) == 4:                                 
            serviceCount += 1
            connectionSocket.send(str(serviceCount).encode())

        
        
        # Exception for invalid option.
        else :
            continue
        
# when user terminate server by ctrl + c
except KeyboardInterrupt:
    try :
        connectionSocket.close()
        serverSocket.close()
        print('\nBye bye~~')
        sys.exit(1)
    # user terminate server when there is no accept,
    except NameError:
        serverSocket.close()
        print('\nBye bye~~')
        sys.exit(1)