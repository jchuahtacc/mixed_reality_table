import socket
import sys

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('127.0.0.1', 4444);

try:
    print("receiving:")
    data, server = sock.recvfrom(4)
    print(data)
finally:
    print("Closing connection")
    sock.close()
