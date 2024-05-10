import socket

SERVER_BROADCAST_PORT = 6970
SERVER_PORT = 6969

SERVER_ALIVE_MESSAGE = 'I am alive!'

def search_for_server() -> socket.socket:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('', SERVER_BROADCAST_PORT))

    data, addr = sock.recvfrom(1024)

    if data.decode() == SERVER_ALIVE_MESSAGE:
        print('Server found!')
        sock.close()
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((addr[0], SERVER_PORT))

    return sock


def client():
    sock = search_for_server()
    server_message = sock.recv(1024)
    print(server_message.decode('utf-8'))
    sock.close()

if __name__ == '__main__':
    client()