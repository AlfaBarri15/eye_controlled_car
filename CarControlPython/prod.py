from wifi_communicator import WiFiCommunicator
from car_commander import CarCommander
from eye_detector import EyeDetector

IP_OPTIONS = ["10.0.0.198", "172.20.10.14", "10.0.0.91", "172.20.10.13"]
CAMERA_OPTIONS = ["Webcam intégrée", "Caméra USB"]


def main():

    print("Veuillez entrer une option")
    for i, option in enumerate(IP_OPTIONS):
        print("{} - {}".format(i+1,option))

    ip_option = int(input()) - 1

    if (ip_option < 0) or (ip_option > len(IP_OPTIONS)-1) or not isinstance(ip_option,int):
        print("Erreur dans l'entrée.")
        exit(1)

    print("Veuillez entrer une option de caméra")
    for i, option in enumerate(CAMERA_OPTIONS):
        print("{} - {}".format(i + 1, option))

    camera_option = int(input())

    if (camera_option < 1) or (camera_option > len(CAMERA_OPTIONS)) or not isinstance(camera_option, int):
        print("Erreur dans l'entrée.")
        exit(1)

    communicator = WiFiCommunicator(ip=IP_OPTIONS[ip_option], port=4099)
    car_commander = CarCommander(communicator=communicator)
    eye_detector = EyeDetector(car_commander=car_commander, camera_option=camera_option)

    # Main loop
    eye_detector.main_loop()


if __name__ == '__main__':
    main()