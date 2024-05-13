import socket
import cmd
import readline
import select
import sys

SERVER_PORT = 6969

TIMEOUT = 5

SERVER_ALIVE_MESSAGE = 'I am alive!'

GET_TEMP_MESSAGE = 'G'.encode('utf-8')

def exit(msg):
    print(msg)
    sys.exit(1)
    
def recv_msg(sock) -> str:
    ready_to_read, _, _ = select.select([sock], [], [], TIMEOUT)
    if ready_to_read:
        data = sock.recv(1024)
        if not data:
            sock.close()
            exit('Server closed the connection!')
        return data.decode()
    else:
        sock.close()
        exit('Server timed out!')

class ClientShell(cmd.Cmd):
    intro = 'Welcome to the client shell! Type help or ? to list commands.\n'
    prompt = '> '

    def __init__(self, sock):
        super().__init__()
        self.sock = sock

    def do_get_temp(self, arg):
        '''
        Get the temperature from the server.
        '''
        self.sock.sendall(GET_TEMP_MESSAGE)
        print('Temperature:', recv_msg(self.sock))

    def do_stop_advertizing(self, arg):
        '''
        Tell the server to stop advertizing. 
        If the connection is lost, the server will start advertizing again.
        '''
        self.sock.sendall('S'.encode('utf-8'))
        recv_msg(self.sock)
        print("Server stopped advertizing.")    

    def do_exit(self, arg):
        '''
        Exit the shell and close the connection.
        '''
        print("Exiting...")
        return True


def search_for_server() -> socket.socket:
    print('Searching for server...')

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('', SERVER_PORT))
    sock.settimeout(20)

    try:
        data, addr = sock.recvfrom(1024)
    except socket.timeout:
        exit('No server found in time!')

    sock.close()
    if data.decode() == SERVER_ALIVE_MESSAGE:
        print('Server found!')
    else:
        exit('No server found!')

    print('Connecting to server...')

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((addr[0], SERVER_PORT))

    message = sock.recv(1024)
    print('Server:', message.decode())
    if message.decode() == 'Only one client allowed at a time':
        exit('Server is busy!')

    return sock


def client():
    sock = search_for_server()
    shell = ClientShell(sock)
    shell.cmdloop()
    sock.close()


if __name__ == '__main__':
    client()