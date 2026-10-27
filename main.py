import cv2
import mediapipe as mp 
import pyautogui
import time
import math
import numpy as np

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

screen_width, screen_height = pyautogui.size()

cap = cv2.VideoCapture(0)
cap.set(3, 640)
cap.set(4, 480)

pyautogui.FAILSAFE = False
pyautogui.PAUSE = 0.01

PALM_IDS = [0, 5, 9, 13, 17]

DEAD_ZONE = 10

PINCH_START = 30

X_MIN, X_MAX = 100, 540
Y_MIN, Y_MAX = 80, 400

def main():
    prev_x, prev_y = screen_width // 2, screen_height // 2
    is_clicking = False
    with mp_hands.Hands(
        max_num_hands=1,
        min_detection_confidence=0.7,
        min_tracking_confidence=0.7,
    ) as hands:

        while True:
            attempt = 0
            success, img = cap.read()
            while not success and attempt < 5:
                time.sleep(0.2)
                success, img = cap.read()
                attempt += 1
            if not success:
                print("Failed to read frame")
                break

            img = cv2.flip(img, 1)
            h, w, _ = img.shape
            rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
            results = hands.process(rgb)

            if results.multi_hand_landmarks:
                for hand_landmarks in results.multi_hand_landmarks:

                    index_tip = hand_landmarks.landmark[8]
                    thumb_tip = hand_landmarks.landmark[4]

                    mp_draw.draw_landmarks(
                        img,
                        hand_landmarks,
                        mp_hands.HAND_CONNECTIONS,
                    )

                    finger_tips = {
                        "Thumb": hand_landmarks.landmark[4],
                        "Index": hand_landmarks.landmark[8],
                        "Middle": hand_landmarks.landmark[12],
                        "Ring": hand_landmarks.landmark[16],
                        "Pinky": hand_landmarks.landmark[20],
                    }

                    for name, landmark in finger_tips.items():
                        x, y = int(landmark.x * w), int(landmark.y * h)
                        
                        cv2.putText(
                            img,
                            name,
                            (x, y -10),
                            cv2.FONT_HERSHEY_SIMPLEX,
                            0.5,
                            (255, 105, 180),
                            1
                        )

                        cv2.circle(
                            img,
                            (x, y),
                            5,
                            (0, 255, 0),
                            -1
                        )

                cx = sum(hand_landmarks.landmark[id].x for id in PALM_IDS) / len(PALM_IDS)
                cy = sum(hand_landmarks.landmark[id].y for id in PALM_IDS) / len(PALM_IDS)


                hand_px = int(cx * w)
                hand_py = int(cy * h)

                screen_x = int(np.interp(hand_px, [X_MIN, X_MAX], [0, screen_width]))
                screen_y = int(np.interp(hand_py, [Y_MIN, Y_MAX], [0, screen_height]))

                dx = screen_x - prev_x
                dy = screen_y - prev_y

                if math.hypot(dx, dy) > DEAD_ZONE:
                    pyautogui.moveTo(
                                        screen_x,
                                        screen_y,
                                        duration=0
                                    ) 
                    prev_x = screen_x
                    prev_y = screen_y  

                ix, iy = int(index_tip.x * w), int(index_tip.y * h)
                tx, ty = int(thumb_tip.x * w), int(thumb_tip.y * h)

                pinch_distance = math.hypot(ix - tx, iy - ty)

                # Als vingers dicht genoeg bij elkaar zijn EN we hebben nog niet geklikt
                if pinch_distance < PINCH_START:
                    if not is_clicking:
                        pyautogui.click()
                        is_clicking = True  # Vergrendel de klik
                        print("Click!")
                else:
                    is_clicking = False  # Reset zodra je je vingers weer opent



            cv2.rectangle(img, (X_MIN, Y_MIN), (X_MAX, Y_MAX), (0, 255, 0), 2)

            cv2.imshow('Image', img)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == "__main__":
    main()