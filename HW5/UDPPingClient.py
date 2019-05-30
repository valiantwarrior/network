# UDPPingClient.py
# 20134220 Jeong hyun, Woo

import socket
import queue
import sys
import getopt
import time
import math

server_ip = "127.0.0.1"
server_port = 24220
timeout = 2.5
sent = 0
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(timeout)
packet_queue = queue.Queue()
receive_observer = []



# command line argument is invalid, print guide for user
def help_for_cmd() :
    print("<OPTION LIST & USAGE>")
    print("-c < IP address  > : Set IP address of the UDP ping server. If not specified, default is 127.0.0.1.")
    print("-p < Port number > : Set Port number of the UDP ping server. If not specified, default is 24220.")
    print("-w < timeout(ms) > : Set the timeout time. If not specified, default is 1000ms.")

def print_result() :
    global receive_observer
    rtt_sum = 0
    rtt_min = 30000
    rtt_max = 0
    received = 0

    for target in receive_observer :
        if target["IS_RECEIVED"] :
            received += 1
        if target["RTT"] >= 0 :
            rtt_sum += target["RTT"]
            if target["RTT"] < rtt_min :
                rtt_min = target["RTT"]
            if target["RTT"] > rtt_max :
                rtt_max = target["RTT"]
    if rtt_min == 30000 :
        rtt_min = 0
    rtt_avg = round(rtt_sum / received) if received > 0 else 0
    print("<Overall Result>")
    print("1. Ping sent         : ", sent)
    print("2. Ping received     : ", received)
    print("3. Ping lost         : ", sent-received)
    print("4. Ping loss ratio   : ", (sent-received) / sent)
    print("5. min RTT           : ", rtt_min)
    print("6. max RTT           : ", rtt_max)
    print("7. avg RTT           : ", rtt_avg)


# define command line argument
def setup_with_command() :
    global server_ip, server_port, sock, timeout
    is_c = 0
    is_p = 0
    is_w = 0

    try :
        option_list, args = getopt.getopt(sys.argv[1:],"c:p:w:")
        if option_list :
            for option in option_list :
                # for each options, Need to check if option is duplicated
                if '-c' in option :
                    if is_c == 1:
                        help_for_cmd()
                        sys.exit(1)
                    server_ip = option[1]
                    is_c = 1
                    continue
                if '-p' in option :
                    if is_p == 1:
                        help_for_cmd()
                        sys.exit(1)
                    server_port = int(option[1])
                    is_p = 1
                    continue
                if '-w' in option :
                    if is_w == 1:
                        help_for_cmd()
                        sys.exit(1)
                    timeout = float(option[1]) / 1000
                    sock.settimeout(timeout)
                    is_w = 1
                    continue 

    except getopt.GetoptError : 
        help_for_cmd()
        sys.exit(1)
    
    # Handling case like '-w nsl2.cau.ac.kr'
    except ValueError :
        help_for_cmd()
        sys.exit(1)


# initialize packet_queue  
def init_packet() :
    for i in range(0,10) :
        packet_queue.put_nowait(i)

# initialize receive_observer       
def init_receive_observer() :
    for i in range(0,10) :
        data = {"PACKET":i, "IS_RECEIVED" : None, "RTT":-1}
        receive_observer.append(data)

# send
def send_to_server(_packet) :
    global sent
    sock.sendto(str(_packet).encode(), (server_ip, server_port))
    begin = time.time()
    
    sent += 1
    print("PING<{0}> sent".format(_packet))
   
    return begin
    
# receive
def receive_from_server(have_to_receive, begin) :
    global receive_observer
    
    try :
        data, addr = sock.recvfrom(64)
        end = time.time()
        data = int(data.decode())

        if data == have_to_receive :
            target = receive_observer[have_to_receive]
            target["RTT"] = math.floor((end - begin) * 1000)
            target["IS_RECEIVED"] = True
            print("PING<{0}> reply received from <{1}> : RTT = <{2}>ms".format(data, addr[0], target["RTT"]))
            return True
        
        else : 
            return False
    
    # timeout
    except socket.timeout : 
        print("PING<{0}> timeout!".format(have_to_receive))
        return False


# For handling timeout case
def receive_clear(have_to_receive) :
    global timeout
    if timeout >= 2.0 :
        sock.settimeout(timeout - 2.0)
    else :
        sock.settimeout(2.0 - timeout)
    
    while True :
        try :
            data, addr = sock.recvfrom(64)
            if int(data.decode()) == have_to_receive :
                sock.settimeout(timeout)
                break
        except  socket.timeout :
            sock.settimeout(timeout)
            break

# packet send and recv
def ping_simulation(_packet) :
    begin = send_to_server(_packet)
    is_received = receive_from_server(_packet,begin)
    
    if is_received is False :
        receive_clear(_packet)

# tell to server that client is terminated
def end_ping_test() :
    sock.sendto("".encode(), (server_ip, server_port))

# main
def main() :

    try :
        
        setup_with_command()
        init_packet()
        init_receive_observer()
        
        while True :
            try :
                packet = packet_queue.get_nowait()
            except queue.Empty :
                break
            ping_simulation(packet)   
        end_ping_test()
        sock.close()
        print_result()

    except KeyboardInterrupt :
        # tell to server that client is terminated
        sock.sendto("".encode(), (server_ip, server_port))
        sock.close()
        print_result()
        sys.exit(1)

if __name__ == "__main__" :
    main()