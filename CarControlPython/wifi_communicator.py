import socket


class WiFiCommunicator:

    def __init__(self, ip: str = '0.0.0.0', port: int = 11111) -> None:
        # Client info
        self._client = None
        self._client_address = None
        self.ip = ip
        self.port = port

        # Socket creation
        self.soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.soc.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

        print("Attempting to connect to {}...".format(ip))
        self.soc.connect((ip, port))
        print("connected to ESP32")

    def reconnect(self):
        self.soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.soc.connect((self.ip, self.port))

    def send_message(self, msg: str = "s"):
        self.soc.sendall(msg.encode('utf-8'))
