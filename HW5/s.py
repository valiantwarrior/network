# UDPPingClient.py
# 20134220 Jeong hyun, Woo

import socket
import time
import sys
import random
import queue
import threading


server_ip = "127.0.0.1"
server_port = 24220
server_addr = (server_ip, server_port)

processing_queue = []

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(server_addr)
lock = threading.Lock()


def processing_simulator() :
    delay = random.random() * 2.
    time.sleep(delay)
    

def packet_loss_simulator() :
    #return False
    return True if random.random() >= 0.8 else False


def ping_simulator(data, client_addr) :
    
    print("From {0} : recv Ping{1}".format(client_addr, data.decode()))    
    
    if packet_loss_simulator() is False :
        processing_simulator()
        sock.sendto(data, client_addr)
        print("To   {0} : reply Ping{1}".format(client_addr, data.decode()))  
    else :
        print("To   {0} : Ping{1} dropped".format(client_addr, data.decode()))

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
    print("BYE!")
  
    

def main() :
   
    try :
        while True:
            data, client_addr = sock.recvfrom(64)

            if client_addr in processing_queue :
                lock.acquire()
                processing_queue[processing_queue.index(client_addr)+1].put_nowait(data)
                lock.release()

            else :
                lock.acquire()
                processing_queue.append(client_addr)
                processing_queue.append(queue.Queue())
                processing_queue[-1].put_nowait(data)
                lock.release()
                threading.Thread(target=multi_client_handler, args=(client_addr,), daemon=True).start()
            #ping_simulator(data,client_addr)
    
    except KeyboardInterrupt :
        sock.close()
        print("Bye~")
        sys.exit(1)

if __name__ == "__main__" :
    main()