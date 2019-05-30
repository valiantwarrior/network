# UDPPingServer.py
# 20134220 Jeong hyun, Woo

import socket
import time
import sys
import random
import queue
import threading

server_port = 24220

# need for multi-client handling
# processing_queue : [client1_addr, queue_for_client1, client2_addr, queue_for_client2,...]
# If client1's address is stored in processing_queue[n](n % 2 == 0), queue for client1 is stored in processing_queue[n+1]
processing_queue = []

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('',server_port))
lock = threading.Lock()

# make delay
def processing_simulator() :
    delay = random.random() * 2.
    time.sleep(delay)

# do packet loss in probability 20%
def packet_loss_simulator() :
    return True if random.random() >= 0.8 else False

# packet recv and send
def ping_simulator(data, client_addr) :
    
    print("From {0} : recv Ping{1}".format(client_addr, data.decode()))    
    
    if packet_loss_simulator() is False :
        processing_simulator()
        sock.sendto(data, client_addr)
        print("To   {0} : reply Ping{1}".format(client_addr, data.decode()))  
    else :
        print("To   {0} : Ping{1} dropped".format(client_addr, data.decode()))

# thread for each client
def multi_client_handler(client_addr) :
    
    while True :
        
        if processing_queue[processing_queue.index(client_addr) + 1].qsize() != 0 :
            lock.acquire()
            data = processing_queue[processing_queue.index(client_addr)+1].get_nowait()
            lock.release()
            if data.decode() == "" :
                break
            ping_simulator(data,client_addr)
        
    lock.acquire()
    del processing_queue[processing_queue.index(client_addr)+1]
    del processing_queue[processing_queue.index(client_addr)]
    lock.release()


def main() :
   
    try :
        while True:
            data, client_addr = sock.recvfrom(64)

            # To handle multi clients, 
            # give 'unique queue' to each client.
            # when data is come from 'client_addr', 
            # check 'client_addr' and put data into corresponding 'unique queue'.

            # if 'unique queue' corresponding 'client_addr' is exist,
            if client_addr in processing_queue :
                lock.acquire()
                processing_queue[processing_queue.index(client_addr)+1].put_nowait(data)
                lock.release()

            # if 'unique queue' corresponding 'client_addr' is not exist,
            # assign 'unique queue' for 'client_addr'
            else :
                lock.acquire()
                processing_queue.append(client_addr)
                processing_queue.append(queue.Queue())
                processing_queue[-1].put_nowait(data)
                lock.release()
                threading.Thread(target=multi_client_handler, args=(client_addr,), daemon=True).start()
    
    except KeyboardInterrupt :
        sock.close()
        print("Bye Bye~")
        sys.exit(1)

if __name__ == "__main__" :
    main()