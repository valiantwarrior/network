import select
from socket import *
from datetime import datetime
import sys
import queue
import time

serviceCount = 0
serverPort = 24220
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.setblocking(0)
serverSocket.bind(('',serverPort))

print('The server is ready to receive on port {0}'.format(serverPort))
serverSocket.listen(5)

clientId = 1

inputs = [serverSocket]
clientInfo = {}
outputs = []
message = {}

try :
    timeout = 3
    while inputs :
        
        start = time.time()
        readable, writable, exceptional = select.select(inputs, outputs, inputs, timeout)
        timeout -= (time.time() - start)
        if not(readable or writable or exceptional) :
            print('Client number : {0}'.format(len(inputs)-1))
            continue

        for socket in readable :
            if socket is serverSocket :

                connectionSocket, clientAddress = socket.accept()
                print('Connection requested from', clientAddress)
                connectionSocket.setblocking(0)
                inputs.append(connectionSocket)
                clientInfo[connectionSocket] = clientId
                clientId += 1
                message[connectionSocket] = queue.Queue()
                print('Client {0} connected. Number of connected clients = {1}'.format(clientInfo[connectionSocket],len(inputs)-1))
            
            else :
                
                data = socket.recv(2048).decode()
                if len(data) == 1:
                    print('Client {0} Command {1}'.format(clientInfo[socket], data))
                    if int(data) == 1 :
                    
                        message[socket].put(data)
                        
                    elif int(data) == 2 :
                        
                        message[socket].put(data)
                        if socket not in outputs :
                            outputs.append(socket)
                    
                    elif int(data) == 3 :
                        
                        message[socket].put(data)
                        if socket not in outputs :
                            outputs.append(socket)

                    elif int(data) == 4 :
                        
                        message[socket].put(data)
                        if socket not in outputs :
                            outputs.append(socket)

                    elif int(data) == 5 :
                        
                        if socket in outputs :
                            outputs.remove(socket)
                        inputs.remove(socket)
                        socket.close()
                        del message[socket]
                        print('Client {0} desconnected. Number of connected clients = {1}'.format(clientInfo[socket],len(inputs)-1))
                        del clientInfo[socket]
                
                    
                    else :
                        continue
            
                
                else :
                    if data == '' :
                        print('Client {0} Command 5'.format(clientInfo[socket]))
                        if socket in outputs :
                            outputs.remove(socket)
                        inputs.remove(socket)
                        socket.close()
                        del message[socket]
                        print('Client {0} desconnected. Number of connected clients = {1}'.format(clientInfo[socket],len(inputs)-1))
                        del clientInfo[socket]
                    
                    else :
                        message[socket].put(data)
                        if socket not in outputs :
                            outputs.append(socket)

        
        for socket in writable :
            try :
                next_msg = message[socket].get_nowait()

            except queue.Empty :
                outputs.remove(socket)

            else :
                
                if int(next_msg) == 1 :
                    socket.send(message[socket].get_nowait().upper().encode())
                    serviceCount += 1

                elif int(next_msg) == 2 :
                    socket.send((clientAddress[0]+','+str(clientAddress[1])).encode())
                    serviceCount += 1

                elif int(next_msg) == 3 :
                    serverTime = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                    socket.send(str(serverTime).encode())
                    serviceCount += 1

                elif int(next_msg) == 4 :
                    serviceCount += 1
                    socket.send(str(serviceCount).encode())

                else :
                    continue

        for socket in exceptional :
            print('handling exceptional condition for {0}'.format(socket.getpeername()))
            inputs.remove(socket)
            if socket in outputs :
                outputs.remove(socket)
            socket.close()
            del message[socket]

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