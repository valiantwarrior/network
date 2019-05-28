#
# 20134220 Jeong hyun, Woo
# BasicUDPClient.py
#

from socket import *
import time
import sys

def printMenu() :
    print('<Menu>')
    print('1) convert text to UPPER-case')
    print('2) get my IP address and port number')
    print('3) get server time')
    print('4) get server service count')
    print('5) exit')

currentTime = lambda : int(time.time() * 1000)


serverName = '165.194.35.202'
serverPort = 24220

clientSocket = socket(AF_INET, SOCK_DGRAM)
clientSocket.bind(('', 5432))


print("The client is running on port", clientSocket.getsockname()[1])

while True :
    printMenu()
    # User enter 'Ctrl + C' or 'Ctrl + Z' -> Program shutdown with 'Bye bye~~' message.
    try :
        option = input('Input option: ')
    except KeyboardInterrupt :
        clientSocket.close()
        print('Bye bye~~')
        break
    except EOFError :
        clientSocket.close()
        print('Bye bye~~')
        break

    try :
        # Option 1 : Convert text to UPPER-case.
        if int(option) == 1:

            start = currentTime()
            clientSocket.sendto(option.encode(), (serverName, serverPort))
            elapsed = currentTime() - start 

            message = input('Input lowercase sentece: ')

            start = currentTime()
            clientSocket.sendto(message.encode(), (serverName, serverPort))
            modifiedMessage, serverAddress = clientSocket.recvfrom(2048)
            elapsed += (currentTime() - start)   

            print('\nReply from server:', modifiedMessage.decode())
            print('Response time:',elapsed,'ms\n')
        
        # Option2 : Get User IP address and port number.
        elif int(option) == 2:

            start = currentTime()
            clientSocket.sendto(option.encode(), (serverName, serverPort))
            info, serverAddress = clientSocket.recvfrom(2048)
            elapsed = currentTime() - start  

            (ip, port) = info.decode().split(',')
            print('\nReply from server:','IP=',ip,'port=',port)
            print('Response time:',elapsed,'ms\n')
        
        # Option3 : Get server's current time.
        elif int(option) == 3:

            start = currentTime()
            clientSocket.sendto(option.encode(), (serverName, serverPort))
            serverTime, serverAddress = clientSocket.recvfrom(2048)
            elapsed = currentTime() - start
            print('\nReply from server:',serverTime.decode())
            print('Response time:',elapsed,'ms\n')
        
        # Option4 : Get server's service count.
        elif int(option) == 4:
            start = currentTime()
            clientSocket.sendto(option.encode(), (serverName, serverPort))
            serviceCount, serverAddress = clientSocket.recvfrom(2048)
            elapsed = currentTime() - start
            print('\nReply from server:',serviceCount.decode())
            print('Response time:',elapsed,'ms\n')
        
        # Option5 : Program terminate.
        elif int(option) == 5:
            clientSocket.sendto(option.encode(), (serverName, serverPort))
            clientSocket.close()
            print('Bye bye~~')
            sys.exit(1)

        else :
            continue

    # if command is invalid form, get new input from user
    except ValueError :
        continue