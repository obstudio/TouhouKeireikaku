import socket

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.bind(('127.0.0.1', 2047))
server.listen(10)

while True:
	print("Waiting for connection...")
	client, addr = server.accept()
	print("%s:%s successfully connected!" % (addr[0], addr[1]))
	while True:
		data = client.recv(1024)
