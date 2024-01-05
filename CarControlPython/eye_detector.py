from car_commander import CarCommander
import base64
import cv2
import numpy as np
import requests
import os
import keyboard as kb

API_KEY = os.environ["ROBOFLOW_API_KEY"]
DISTANCE_TO_OBJECT = 650  # mm
HEIGHT_OF_HUMAN_FACE = 250  # mm
GAZE_DETECTION_URL = "http://127.0.0.1:9001/gaze/gaze_detection?api_key=" + API_KEY
TOLERANCE = 60


class EyeDetector:

    def __init__(self, car_commander: CarCommander, camera_option: int = 1):
        self._car_commander = car_commander
        self.camera_option = camera_option-1
        self.main_loop()

    def main_loop(self):
        cap = cv2.VideoCapture(self.camera_option)

        while True:
            move_command = False    # Reset at every loop

            if kb.is_pressed('up'):
                self._car_commander.go_forward()
                move_command = True

            if kb.is_pressed('down'):
                self._car_commander.go_backward()
                move_command = True

            _, frame = cap.read()

            gazes = self.detect_gazes(frame)

            if len(gazes) == 0:
                if not move_command:
                    self._car_commander.stop()
                continue

            gaze = gazes[0]

            image_height, image_width = frame.shape[:2]

            length_per_pixel = HEIGHT_OF_HUMAN_FACE / gaze["face"]["height"]

            dx = -DISTANCE_TO_OBJECT * np.tan(gaze['yaw']) / length_per_pixel
            # 100000000 is used to denote out of bounds
            dx = dx if not np.isnan(dx) else 100000000
            line_position = int(image_width / 2 + dx)

            cv2.line(frame, (line_position, 0), (line_position, image_height), (0, 255, 0), 2)

            if not move_command:
                move_command = self.command_direction(dx)

            frame = self.display_gazepoint_coordinates(frame, line_position)

            cv2.imshow("Belle personne", frame)

            if cv2.waitKey(1) & 0xFF == ord("q"):
                break

            if not move_command:
                self._car_commander.stop()

    def detect_gazes(self, frame: np.ndarray):
        img_encode = cv2.imencode(".jpg", frame)[1]
        img_base64 = base64.b64encode(img_encode)
        resp = requests.post(
            GAZE_DETECTION_URL,
            json={
                "api_key": API_KEY,
                "image": {"type": "base64", "value": img_base64.decode("utf-8")},
            }
        )
        gazes = resp.json()[0]["predictions"]
        return gazes

    def draw_gaze(self, img: np.ndarray, gaze: dict):
        # draw face bounding box
        face = gaze["face"]
        x_min = int(face["x"] - face["width"] / 2)
        x_max = int(face["x"] + face["width"] / 2)
        y_min = int(face["y"] - face["height"] / 2)
        y_max = int(face["y"] + face["height"] / 2)
        cv2.rectangle(img, (x_min, y_min), (x_max, y_max), (255, 0, 0), 3)

    def show_command_direction(self, x: int) -> str:
        if x < -TOLERANCE:
            return "droite"
        elif x > TOLERANCE:
            return "gauche"
        else:
            return "centre"

    def command_direction(self, x: int) -> bool:
        if x < -TOLERANCE:
            self._car_commander.go_right()
            return True
        elif x > TOLERANCE:
            self._car_commander.go_left()
            return True
        else:
            return False

    def display_gazepoint_coordinates(self, image, x):
        height, width = image.shape[:2]

        # Define the text and its position
        text = f'X: {int(x)}, Dir: {self.show_command_direction(x)}'
        org = (width - 200, height - 20)  # Adjust these values as needed

        # Choose the font and its properties
        font = cv2.FONT_HERSHEY_SIMPLEX
        font_scale = 0.5
        font_thickness = 1
        font_color = (255, 255, 255)  # White color

        # Put the text on the image
        cv2.putText(image, text, org, font, font_scale, font_color, font_thickness, cv2.LINE_AA)

        return image
