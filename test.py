import socket

address = "./access_socket"
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
sock.connect(address)
sock.send('openDoor\0')
data = sock.recv(512)
print(data)
