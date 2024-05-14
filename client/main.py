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
    temp_min = -20
    temp_max = 60
    humidity_min = 5
    humidity_max = 95
    
    def __init__(self, sock):
        super().__init__()
        self.sock = sock
        self.temperatureType = str(recv_msg(self.sock))
    
    def conv_temp(self, temperature):
        if self.temperatureType == 'F':
            return int(temperature * 9 / 5 + 32)
        elif self.temperatureType == 'K':
            return int(temperature + 273.15)
        return temperature
    
    def reverse_temp(self, temperature):
        if self.temperatureType == 'F':
            return int((temperature - 32) * 5 / 9)
        elif self.temperatureType == 'K':
            return int(temperature - 273.15)
        return temperature

    def do_get_temp(self, arg):
        '''
        Get the temperature from the server.
        '''
        send_msg(self.sock, GET_TEMP)
        print(f'Temperature: {self.conv_temp(int(recv_msg(self.sock)))}{self.temperatureType}')
        
    def do_get_humidity(self, arg):
        '''
        Get the humidity from the server.
        '''
        send_msg(self.sock, GET_HUMIDITY)
        print(f'Humidity:{recv_msg(self.sock)}')

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
        
    def help_set_temp_threshold(self):
        return f'''
        Set the temperature threshold on the server.
        Usage: set_temp_threshold <temperature({self.conv_temp(self.temp_min)}, {self.conv_temp(self.temp_max)}){self.temperatureType}>
        '''

    def do_set_temp_threshold(self, arg):
        f'''
        Set the temperature threshold on the server.
        Usage: set_temp_threshold <temperature({self.conv_temp(self.temp_min)}, {self.conv_temp(self.temp_max)}){self.temperatureType}>
        '''
        try:
            temp = int(arg)
            if not self.conv_temp(self.temp_min) <= temp <= self.conv_temp(self.temp_max):
                print('Temperature must be in specified range! Type help for more info.')
                return
        except ValueError:
            print('Invalid temperature!')
            return
        send_msg(self.sock, SET_TEMP_THRESHOLD + str(self.reverse_temp(temp)))
        print(f'Temperature threshold set to: {self.conv_temp(recv_msg(self.sock))}{self.temperatureType}')

    def help_set_humidity_threshold(self):
        return f'''
        Set the temperature threshold on the server.
        Usage: set_humidity_threshold <humidity({self.humidity_min}, {self.humidity_max})>%
        '''

    def do_set_humidity_threshold(self, arg):
        try:
            humidity = int(arg)
            if humidity < self.humidity_min or humidity > self.humidity_max:
                print('Humidity must be in specified range! Type help for more info.')
                return
        except ValueError:
            print('Invalid humidity!')
            return
        send_msg(self.sock, SET_HUMIDITY_THRESHOLD + str(humidity))
        print(f'Humidity threshold set to: {recv_msg(self.sock)}%')
    
    def help_set_motor_interval(self):
        return f'''
        Set the motor interval on the server.
        Usage: set_motor_interval <low({self.conv_temp(self.temp_min)}, {self.conv_temp(self.temp_max)}){self.temperatureType}> \
        <high({self.conv_temp(self.temp_min)}, {self.conv_temp(self.temp_max)}){self.temperatureType}>
        '''
    
    def do_set_motor_interval(self, arg):
        try:
            low, high = map(int, arg.split())
            if low > high:
                print('Low temperature must be less than high temperature!')
                return
            if ((not self.conv_temp(self.temp_min) <= low <= self.conv_temp(self.temp_max)) or
                (not self.conv_temp(self.temp_min) <= high <= self.conv_temp(self.temp_max))):
                print('Temperature must be in specified range! Type help for more info.')
                return
        except ValueError:
            print('Invalid temperature!')
            return
        send_msg(self.sock, SET_MOTOR_INTERVAL + str(self.reverse_temp(low)) + '_' + str(self.reverse_temp(high)))
        server_low, server_high = map(int, recv_msg(self.sock).split('_'))
        print(f'Motor interval set to: ({self.conv_temp(server_low)}-{self.conv_temp(server_high)}{self.temperatureType})')

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
        print(f'Temperature type set to: {recv_msg(self.sock)}')

    def do_exit(self, arg):
        '''
        Exit the shell and close the connection.
        '''
        print("Exiting...")
        return True
    
    def do_help(self, arg):
        '''
        Get help for a command. Type help <command> for more info.
        '''
        if arg == 'set_motor_interval':
            print(self.help_set_motor_interval())
        elif arg == 'set_temp_threshold':
            print(self.help_set_temp_threshold())
        elif arg == 'set_humidity_threshold':
            print(self.help_set_humidity_threshold())
        else:
            super().do_help(arg)

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