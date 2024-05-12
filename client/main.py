import socket
import cmd
import readline
import select

SERVER_BROADCAST_PORT = 6970
SERVER_PORT = 6969

TIMEOUT = 3

SERVER_ALIVE_MESSAGE = 'I am alive!'

GET_TEMP_MESSAGE = 'GET_TEMPERATURE'.encode('utf-8')

class ClientShell(cmd.Cmd):
    intro = 'Welcome to the client shell! Type help or ? to list commands.\n'
    prompt = '> '

    def __init__(self, sock):
        super().__init__()
        self.sock = sock

    def do_get_temp(self, arg):
        'Get the temperature from the server.'
        self.sock.sendall(GET_TEMP_MESSAGE)
        ready_to_read, _, _ = select.select([self.sock], [], [], TIMEOUT)
        if ready_to_read:
            data = self.sock.recv(1024)
            print('Temperature:', data.decode())
        else:
            print('No response from server')

    def do_exit(self, arg):
        'Exit the shell and close the connection.'
        print("Exiting...")
        return True


def search_for_server() -> socket.socket:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('', SERVER_BROADCAST_PORT))

    data, addr = sock.recvfrom(1024)

    if data.decode() == SERVER_ALIVE_MESSAGE:
        print('Server found!')
        sock.close()
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((addr[0], SERVER_PORT))
    if sock:
        print('Connected to server!')

    return sock


def client():
    sock = search_for_server()
    shell = ClientShell(sock)
    shell.cmdloop()
    sock.close()


if __name__ == '__main__':
    client()