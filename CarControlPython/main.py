from wifi_communicator import WiFiCommunicator
from car_commander import CarCommander
from eye_detector import EyeDetector


def main():
    communicator = WiFiCommunicator(ip='10.0.0.91', port=4099)
    car_commander = CarCommander(communicator=communicator)
    eye_detector = EyeDetector(car_commander=car_commander)

    # Main loop
    eye_detector.main_loop()


if __name__ == '__main__':
    main()
