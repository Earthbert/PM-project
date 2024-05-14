import socket
import cmd
import readline
import select
import sys

SERVER_PORT = 6969

TIMEOUT = 5

SERVER_ALIVE_MESSAGE = 'I am alive!'

GET_TEMP = 'G'
GET_HUMIDITY = 'H'
STOP_ADVERTISE = 'S'
START_ADVERTISE = 'A'
SET_TEMP_THRESHOLD = 'T'
SET_HUMIDITY_THRESHOLD = 'U'
SET_MOTOR_INTERVAL = 'M'
SET_TEMPERATURE_TYPE = 'P'
GET_CONFIG = 'C'

def exit(msg):
    print(msg)
    sys.exit(1)
    
def recv_msg(sock) -> str:
    ready_to_read, _, _ = select.select([sock], [], [], TIMEOUT)
    if ready_to_read:
        try:
            data = sock.recv(1024)
            if not data:
                sock.close()
                exit('Server closed the connection!')
            return data.decode()
        except socket.error:
            sock.close()
            exit('Server closed the connection!')
    else:
        sock.close()
        exit('Server timed out!')
        
def send_msg(sock, msg):
    try:
        sock.sendall(msg.encode('utf-8'))
    except socket.error:
        exit('Server closed the connection!')

class ClientShell(cmd.Cmd):
    intro = 'Welcome to the client shell! Type help or ? to list commands.\n'
    prompt = '> '
    temperatureType = 'C'
    
    def convert_to_current_temperature(self, temperature):
        if self.temperatureType == 'F':
            return int(temperature * 9 / 5 + 32)
        elif self.temperatureType == 'K':
            return int(temperature + 273.15)
        return temperature

    def __init__(self, sock):
        super().__init__()
        self.sock = sock
        temperatureType = str(recv_msg(self.sock))

    def do_get_temp(self, arg):
        '''
        Get the temperature from the server.
        '''
        send_msg(self.sock, GET_TEMP)
        print('Temperature:', self.convert_to_current_temperature(int(recv_msg(self.sock))))
        
    def do_get_humidity(self, arg):
        '''
        Get the humidity from the server.
        '''
        send_msg(self.sock, GET_HUMIDITY)
        print('Humidity:', recv_msg(self.sock))

    def do_stop_advertizing(self, arg):
        '''
        Tell the server to stop advertizing. 
        If the connection is lost, the server will start advertizing again.
        '''
        send_msg(self.sock, STOP_ADVERTISE)
        recv_msg(self.sock)
        print("Server stopped advertizing.")
    
    def do_start_advertizing(self, arg):
        '''
        Tell the server to start advertizing.
        '''
        send_msg(self.sock, START_ADVERTISE)
        recv_msg(self.sock)
        print("Server started advertizing.")

    def do_set_temp_threshold(self, arg):
        '''
        Set the temperature threshold on the server.
        Usage: set_temp_threshold <temperature(-20, 60)C>
        '''
        try:
            temp = int(arg)
            if temp < -20 or temp > 60:
                print('Temperature must be between -20 and 60!')
                return
        except ValueError:
            print('Invalid temperature!')
            return
        send_msg(self.sock, SET_TEMP_THRESHOLD + str(temp))
        print(recv_msg(self.sock))

    def do_set_humidity_threshold(self, arg):
        '''
        Set the temperature threshold on the server.
        Usage: set_humidity_threshold <humidity(5, 95)>
        '''
        try:
            humidity = int(arg)
            if humidity < 5 or humidity > 95:
                print('Humidity must be between 5 and 95!')
                return
        except ValueError:
            print('Invalid humidity!')
            return
        send_msg(self.sock, SET_HUMIDITY_THRESHOLD + str(humidity))
        print(recv_msg(self.sock))
        
    def do_set_motor_interval(self, arg):
        '''
        Set the motor interval on the server.
        Usage: set_motor_interval <low(-20, 60)C> <high(-20, 60)C>
        '''
        try:
            low, high = map(int, arg.split())
            if low < -20 or low > 60 or high < -20 or high > 60:
                print('Temperature must be between -20 and 60!')
                return
        except ValueError:
            print('Invalid temperature!')
            return
        send_msg(self.sock, SET_MOTOR_INTERVAL + str(low) + '_' + str(high))
        print(recv_msg(self.sock))

    def do_get_config(self, arg):
        '''
        Get the current configuration from the server.
        '''
        send_msg(self.sock, GET_CONFIG)
        print(recv_msg(self.sock))
        
    def do_set_temperature_type(self, arg):
        '''
        Set the temperature type to C, F or K.
        '''
        if arg not in ['C', 'F', 'K']:
            print('Invalid temperature type!')
            return
        self.temperatureType = arg
        send_msg(self.sock, SET_TEMPERATURE_TYPE + arg)
        print(recv_msg(self.sock))

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