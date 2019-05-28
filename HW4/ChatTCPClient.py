from socket import *
import sys
import threading
import _thread
import os
IP = "165.194.35.202"
PORT = 24220
NICKNAME_LENGTH = 64
CLIENT_QUIT = "$!#ZCR!$$$@##!#!"
SERVER_QUIT = "!#$!FFDSSVVVSDG!@!#$!!@##"
VILLAIN_DETECTED = "3##21$!2@3991"

# nickname form check. nickname(<=64) can contain alphabet,nuber and -
def nickname_validation(nickname) :
    
    if(len(nickname) > NICKNAME_LENGTH):
        print("Nickname length limit : 64.")
        return False
    else:
        for char in nickname :
            if char.isalpha() == True :
                continue
            elif char.isdigit() == True :
                continue
            elif char == '-' :
                continue
            else :
                print("Nickname : number,alphabet,- allow.")
                return False
        return True

# Background thread for read from server
def thread_read_func(serverSocket) :
    
    while True :
        
        message = serverSocket.recv(2048).decode().replace('\x00','')
        
        if len(message) > 0 :
            # after client send '\quit', sever recieve it and send "CLIENT_QUIT"
            # if client receive "CLIENT_QUIT", terminate client program
            if CLIENT_QUIT == message :
                print("Bye~")
                serverSocket.close()
                os._exit(os.EX_OK)
            
            # Client receive "SERVER_QUIT", that means server terminated.
            # So terminate client program too.
            if SERVER_QUIT == message :
                print("Server terminated. Bye~")
                serverSocket.close()
                os._exit(os.EX_OK)
            
            # This client try to send "i hate professor" and server catch it.
            # Server send the "VILLAIN_DETECTED" to this client
            # Terminate client 
            if VILLAIN_DETECTED == message :
                print("\033[31mYou are VILLAIN! Bye!")
                serverSocket.close()
                os._exit(os.EX_OK)
            
            # For other case, just show message from others
            print(message,end='')
        
    serverSocket.close()
    sys.exit(1)


###############################################

try :
    # valid argument check
    if len(sys.argv) != 2 :
        print("Wrong argument")
        sys.exit(1)

    # nickname check
    if nickname_validation(sys.argv[1]) == False :
        sys.exit(1)


    serverSocket = socket(AF_INET,SOCK_STREAM,0)
    serverSocket.connect((IP,PORT))

    number_of_client_in_server = 0
    number_of_client_in_server = int(serverSocket.recv(2048).decode())

    # check server's current client number
    # if 8, terminate this client,
    if number_of_client_in_server == 8 :
        print("full.cannot connect")
        serverSocket.close()
        sys.exit(1)

    # else, continuing connection and send nickname to server
    serverSocket.send(sys.argv[1].encode())

    # If nickname is duplicated, close connection
    validation = serverSocket.recv(2048).decode()
    if int(validation) == 1 :
        print("Duplicated Nickname. Cannot connect")
        serverSocket.close()

    # else, continuing conenction
    else :
        # send nickname again to save client information in server
        serverSocket.send(sys.argv[1].encode())

        # receive welcome message from server
        welcome = serverSocket.recv(2048).decode()
        print(welcome)

        # Background thread for read from server
        thread_read = threading.Thread(target=thread_read_func,args=(serverSocket,),daemon=True)
        thread_read.start()
        
        while True :
            
            # Get input
            message = input()+'\n\0'

            # If input start with \,
            if message[0] == '\\' :
                arg = message.split(' ')

                # command : \quit
                if arg[0]== "\\quit\n\0" :
                    serverSocket.send(message.encode())

                # command : \stats
                elif arg[0] == "\\stats\n\0" :
                    serverSocket.send(message.encode())

                # command : \w <nickname> <message>
                elif arg[0] == "\\w" :
                    serverSocket.send(message.encode())

                # command : \change <nickname>
                elif arg[0] == "\\change" :
                    # before send this command, check whether <nickname> is valid form
                    arg[1] = arg[1].replace("\n\0","")
                    if nickname_validation(arg[1]) == True :
                        serverSocket.send(message.encode())
                    else :
                        continue
                # command : \ver
                elif arg[0] == "\\ver\n\0" :
                    serverSocket.send(message.encode())
                
                # command : \list
                elif arg[0] == "\\list\n\0" :
                    serverSocket.send(message.encode())
                
                # Other case : Ignore (Ex> \quit hi \list hi ...)
                else :
                    print("COMMAND LIST :\n \\list\n \\w <nickname> <message>\n \\quit\n \\ver\n \\change <new nickname>\n \\stats")
            
            # input is not command, just chat message    
            else :
                serverSocket.send(message.encode())

        serverSocket.close()
        sys.exit(1)        

# ctrl + c : close socket & print bye
except KeyboardInterrupt :
    try :
        serverSocket.close()
    except NameError :
        pass
    print("\nBye~")
    sys.exit(1)