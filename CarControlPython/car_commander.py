from wifi_communicator import WiFiCommunicator

class CarCommander:

    def __init__(self, communicator: WiFiCommunicator):
        super().__init__()

        # The Wi-Fi communicator object
        self._communicator = communicator

    def go_forward(self):
        self._communicator.send_message('f')
        print("Going forward")

    def go_backward(self):
        self._communicator.send_message('b')
        print("Going backward")

    def go_left(self):
        self._communicator.send_message('l')
        print("Turning left")

    def go_right(self):
        self._communicator.send_message('r')
        print("Turning right")

    def stop(self):
        self._communicator.send_message('s')
        print("Stopped")