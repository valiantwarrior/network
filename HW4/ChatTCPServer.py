from socket import *
import threading
import sys


CLIENT_QUIT = "$!#ZCR!$$$@##!#!"
SERVER_QUIT = "!#$!FFDSSVVVSDG!@!#$!!@##"
VILLAIN_DETECTED = "3##21$!2@3991"

MAX_CLIENT = 8
lock = threading.Lock()

# Data structure for client : [ {client1} {client2} {client3} ...   ]
# For each client, contain
# {
#  "nickname"   : 
#  "ip"         :
#  "port"       :
#  "socket"     :
#  "sendn"      :
#  "receiven"   :
# }

client_info_list = []

# get number of clients currently connected
def get_client_number() :
    global client_info_list
    return len(client_info_list)

# check whether there is client who's nickname is <nickname>
def is_exist(nickname) :
    target = nickname
    global client_info_list
 
    for client in client_info_list :
        if client['nickname'] == target :
            return client
    return None

# detecting "i hate professor"
def villain_dectect(message) :
    
    villain = "ihateprofessor"
    temp = message.lower().replace(" ","")
    if villain in temp :
        return True
    else :
        return False

# add client info in client_info_list[]
def append_clientinfo(connectionSocket,clientAddress) :
    
    global client_info_list

    client_info = {}
    nickname = connectionSocket.recv(2048).decode().replace("\x00","")

    client_info['nickname'] = nickname
    client_info['ip'] = clientAddress[0]
    client_info['port'] = str(clientAddress[1])
    client_info['sockfd'] = connectionSocket
    client_info['sendn'] = 0
    client_info['receiven'] = 0
    client_info_list.append(client_info)

    return client_info

# remove client info from client_info_list[]
def remove_clientinfo(current) :
    
    global client_info_list

    target = current
    if target in client_info_list :
        client_info_list.remove(target)
        
# 1 thread <-> 1 client
def thread_main_func(connectionSocket,clientAddress) :
    
    global client_info_list
    current ={}

    # add new client in client_info_list
    lock.acquire()
    current = append_clientinfo(connectionSocket,clientAddress)
    lock.release()

    # When connecting to C client, the problem is that "recv" for nickname is not properly done in
    # append_clientinfo function.
    # This code is prepared for failing to recv a nickname from client.
    if len(current["nickname"]) == 0 :
        while True :
            nickname = connectionSocket.recv(2048).decode().replace("\x00","")
            if len(nickname) > 0 :
                current["nickname"] = nickname
                break

    print("New client : {0} <{1},{2}> Connected.(N : {3})".format( current["nickname"], current["ip"], current["port"], get_client_number() ))
    
    # Send welcome message to newly connected client
    welcome = "Welcome {0} to cau-cse chat room at <{1},{2}> You are {3}th user.".format(current["nickname"],serverIP,serverPort,get_client_number())
    current["sockfd"].send(welcome.encode())
    
    # Announce new client connected to every client(except this client)
    notice = "[{0} is connected. There are {1} users in the chat room.]\n\0".format(current["nickname"],get_client_number())
    for target in client_info_list :
        if target == current :
            continue
        target["sockfd"].send(notice.encode())

    
    while True :
        
        # Read from client
        message = connectionSocket.recv(2048).decode()
        
        # If client send Ctrl+C, remove this clientinfo from client_info_list and exit this loop for clean-up
        if len(message) <= 0 :
            break

        else :
            message = message.replace('\x00','') 
            if len(message) <= 0 :
                continue

            print("[From : {0}] Read data {1}({2}) : {3}".format(current["nickname"],current["ip"],current["port"],message),end='')
            
            

            # message starting with '\' means that this message is command
            if message[0] == '\\' :
                # if server received \quit, sever send "CLIENT_QUIT" code and exit while loop
                # when client receive this code, client will soon exit program
                if "\\quit" in message :
                    connectionSocket.send(CLIENT_QUIT.encode())
                    break
                
                # \stats
                if "\\stats" in message :
                    info  = "Sent : {0}\nReceived : {1}\n\0".format(current['sendn'],current['receiven'])
                    connectionSocket.send(info.encode())
                
                # \w <nickname> <message>
                if "\\w" in message :
                    args = message.split(' ')
                    if args[0] =='\\w' and args[1] and args[2] :
    
                        target = is_exist(args[1])

                        # if <nickname> exist and this client's nickname != <nickname>
                        # send <meesage> to <nickname>
                        if target and target is not current:
                            message = message.replace(args[0],"")
                            message = message.replace(args[1],"")
                            wMessage = "\033[32m<Whipser {0}> : {1}\033[0m".format(current["nickname"],message)
                            target["sockfd"].send(wMessage.encode())
                            current["sendn"] += 1
                            target["receiven"] += 1
                        
                        # if <nickname> is not exist, notice to this client
                        elif target is None :
                            connectionSocket.send("\033[31mThat user is not exist.\033[0m\n\0".encode())
                        
                        # if <nickname> == this client's nickname, notice to this client
                        elif target is current :
                            connectionSocket.send("\033[31mTalk to yourself?\033[0m\n\0".encode())
                        
                        else :
                            continue   
                # \change <nickname>         
                if "\\change" in message :
                    args = message.split(' ')
                    args[1] = args[1].replace('\n','')
                    
                    # if <nickname> already exist, notice to this client
                    if is_exist(args[1]) :
                        connectionSocket.send(("Nickname <{0}> is already exist.\n\0".format(args[1])).encode())
                    
                    # if <nickname> is not exist, change this client's nickname to <nickname>
                    else :
                        current["nickname"] = args[1]
                        connectionSocket.send( ("Your nickname changed : <{0}>\n\0".format(current["nickname"])).encode() )
                
                # \list
                if "\\list" in message :
                    clientinfo =[]
                    for target in client_info_list :
                        clientinfo.append("<{0} {1} {2}>\n".format(target["nickname"],target["ip"],target["port"]))
                    output = "".join(clientinfo)
                    connectionSocket.send(output.encode())
                
                # \ver
                if "\\ver" in message :
                    connectionSocket.send("Server 1.3v Client 1.1v\n\0".encode())

            # Not start with '\' : Just chat message. Send to everyone.
            else :
                message = "{0}>".format(current["nickname"])+message
                for target in client_info_list :
                    if target is current :
                        target["sendn"] += 1
                        continue
                    target["receiven"] += 1
                    target["sockfd"].send(message.encode())
            
            # censor message
            if villain_dectect(message) == True :
                # send "VILLAN_DETECTED" code to this client, exist while loop
                connectionSocket.send(VILLAIN_DETECTED.encode())
                break

    # exit from while loop : Client \quit or Ctrl+C
    # remove this client info from client_info_list
    # announce to other client that this client is quit
    # close this socket
    # terminate this thread                    
    lock.acquire()            
    remove_clientinfo(current)
    lock.release()
    
    notice = "[{0} is disconnected. There are {1} users in the chat room.]\n\0".format(current["nickname"],get_client_number())
    
    for target in client_info_list :
        if target == current :
            continue
        target["sockfd"].send(notice.encode())

    print("{0} quit".format(current["nickname"]))
    connectionSocket.close()
    sys.exit()
   
    
    
serverIP = "165.194.35.202"
serverPort = 24220
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind(('',serverPort))
serverSocket.listen(5)

try :
    while True :

        (connectionSocket, clientAddress) = serverSocket.accept()
        
        current_client_number = get_client_number()
        # Server sends the number of clients currently connected to the client attempting a new connection
        connectionSocket.send(str(current_client_number).encode())
        
        # If current connected clients == 8, the connection rejected and close socket.
        if current_client_number ==MAX_CLIENT :
            connectionSocket.close()
            continue
        
        # If current connected clients under 8,
        else :
            # client will send nickname to server.
            nickname = connectionSocket.recv(64).decode()
            nickname = nickname.replace("\x00","")
            
            # and if received nickname is duplicated, send 1(If client receive 1, client will exist)
            # + close socket
            if is_exist(nickname) :
                connectionSocket.send(str(1).encode())
                connectionSocket.close()
            
            # if nickname is not duplicated, send 0 (after client receiving 0, continuing connection)
            else :
                connectionSocket.send(str(0).encode())
                # start thread for connection
                thread = threading.Thread(target=thread_main_func,args=(connectionSocket,clientAddress),daemon=True)
                thread.start()

# server terminate with Ctrl+C,
except KeyboardInterrupt :
    try :
        # if there are connected client, send to every clients "SERVER_QUIT",
        # close each socket.
        # if client receive "SERVER_QUIT", client will terminate
        serverSocket.close()
        for target in client_info_list :
            target["sockfd"].send(SERVER_QUIT.encode())
            target["sockfd"].close()
        print("\nBye~")
        sys.exit()
    # no connected client, just close socket.
    except NameError :
        serverSocket.close()
        print("\nBye~")
        sys.exit()  
            

            