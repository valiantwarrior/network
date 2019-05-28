#
# 20134220 Jeong hyun, Woo
# BasicTCPClient.py
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


serverName = '127.0.0.1'
serverPort = 24220

# create socket
clientSocket = socket(AF_INET, SOCK_STREAM)

# connect
clientSocket.connect((serverName, serverPort))
print("The client is running on port", clientSocket.getsockname()[1])

try :

    while True:

        printMenu()

        # User enter 'Ctrl + C' or 'Ctrl + Z' -> Program shutdown with 'Bye bye~~' message.
        try :
            option = input('Input option: ')
        except KeyboardInterrupt as e :
            clientSocket.close()
            print('Bye bye~~')
            break
        except EOFError as e :
            clientSocket.close()
            print('Bye bye~~')
            break

        try :
            # Option 1 : Convert text to UPPER-case.
            if int(option)== 1:
                
                # send command 
                start = currentTime()                                       #
                clientSocket.send(option.encode())                          # response time 1
                elapsed = currentTime() - start                             #

                message = input('Input lowercase sentece: ')
                
                # send sentence & receive uppercase-sentence
                start = currentTime()                                       #
                clientSocket.send(message.encode())                         # response time 2
                modifiedMessage = clientSocket.recv(2048)                   #
                
                elapsed += (currentTime() - start)                          # total response-time = response time1 + response time 2

                print('\nReply from server: ', modifiedMessage.decode())
                print('Response time:',elapsed,'ms\n')
            
            # Option2 : Get User IP address and port number.
            elif int(option) == 2:

                start = currentTime()
                # send command
                clientSocket.send(option.encode())
                # receive client ip & port number
                clientInfo = clientSocket.recv(2048)
            
                elapsed = currentTime() - start # round-trip time                   
                [ip, port] = clientInfo.decode().split(',')
                print('\nReply from server:','IP=',ip,'port=',port)
                print('Response time:',elapsed,'ms\n')
            
            # Option3 : Get server's current time.
            elif int(option) == 3:

                start = currentTime()
                # send command
                clientSocket.send(option.encode())
                # get server's current time
                serverTime = clientSocket.recv(2048)

                elapsed = currentTime() - start # round-trip time

                print('\nReply from server:', serverTime.decode())
                print('Response time:',elapsed,'ms\n')

            # Option4 : Get server's service count.
            elif int(option) == 4:

                start = currentTime()
                # send command
                clientSocket.send(option.encode())
                # receive server's service count
                serviceCount = clientSocket.recv(2048)

                elapsed = currentTime() - start # round-trip time

                print('\nReply from server:client request count =',serviceCount.decode())
                print('Response time:',elapsed,'ms\n')
            
            # Option5 : Program terminate.
            elif int(option) == 5:
                
                clientSocket.send(option.encode())
                print('Bye bye~')
                clientSocket.close()
                sys.exit(1)
            
            else :
                continue
        # if command is invalid form, get new input from user
        except ValueError :
            continue


except ConnectionAbortedError as e:
    clientSocket.close()
    print(e)
    sys.exit(1)

# if user terminate program by Ctrl + C
except KeyboardInterrupt :
    clientSocket.close()
    print('Bye bye~~')
    sys.exit(1)
