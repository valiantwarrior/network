import queue

list =[]


ip = "127.0.0.1"
port = 2222

list.append((ip,port))
list.append(queue.Queue())
list.append((ip,port+3))
list.append(queue.Queue())
list.append((ip,port+1))
list.append(queue.Queue())
list.append((ip,port+2))
list.append(queue.Queue())

if (ip,port) in list :
        list[list.index((ip,port))+1].put_nowait(1)

print(list[3].get_nowait())

print(list)