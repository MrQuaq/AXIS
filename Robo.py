import cv2
import mediapipe as mp 
import time
import math
import numpy as np
import socket

# Setup UDP Socket to transmit to ESP32
ESP32_IP = "192.168.0.115"  # Replace with your ESP32 IP
ESP32_PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

mp_hands = mp.solutions.hands
mp_draw = mp.solutions.drawing_utils

cap = cv2.VideoCapture(0)
cap.set(3, 640)
cap.set(4, 480)

PINCH_START = 35           # Distance threshold to trigger "grab"
JOYSTICK_RADIUS = 120      # Maximum pixel distance from center
MAX_STEER = 50.0           # Maximum steering offset
MAX_TILT = 2.0             # Maximum pitch angle setpoint

def main():
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

            # Center of screen (Joystick origin)
            center_x, center_y = w // 2, h // 2
            point_x, point_y = center_x, center_y

            speed_cmd = 0.0
            steer_cmd = 0.0
            is_pinching = False

            if results.multi_hand_landmarks:
                for hand_landmarks in results.multi_hand_landmarks:
                    # Draw Mediapipe skeleton
                    mp_draw.draw_landmarks(
                        img,
                        hand_landmarks,
                        mp_hands.HAND_CONNECTIONS,
                    )

                    index_tip = hand_landmarks.landmark[8]
                    thumb_tip = hand_landmarks.landmark[4]

                    ix, iy = int(index_tip.x * w), int(index_tip.y * h)
                    tx, ty = int(thumb_tip.x * w), int(thumb_tip.y * h)

                    # Calculate midpoint and distance between thumb and index tip
                    pinch_cx = (ix + tx) // 2
                    pinch_cy = (iy + ty) // 2
                    pinch_distance = math.hypot(ix - tx, iy - ty)

                    # Trigger joystick grab when fingers are pinched together
                    if pinch_distance < PINCH_START:
                        is_pinching = True

                        dx = pinch_cx - center_x
                        dy = pinch_cy - center_y
                        dist_from_center = math.hypot(dx, dy)

                        # Clamp joystick movement inside the circle
                        if dist_from_center > JOYSTICK_RADIUS:
                            angle = math.atan2(dy, dx)
                            dx = int(JOYSTICK_RADIUS * math.cos(angle))
                            dy = int(JOYSTICK_RADIUS * math.sin(angle))

                        point_x = center_x + dx
                        point_y = center_y + dy

                        # Convert displacement into speed tilt and steering commands
                        speed_cmd = np.interp(-dy, [-JOYSTICK_RADIUS, JOYSTICK_RADIUS], [-MAX_TILT, MAX_TILT])
                        steer_cmd = np.interp(dx, [-JOYSTICK_RADIUS, JOYSTICK_RADIUS], [-MAX_STEER, MAX_STEER])

                        # Draw small green indicator at pinch location
                        cv2.circle(img, (pinch_cx, pinch_cy), 6, (0, 255, 0), -1)

            # Transmit UDP message to ESP32: "speed,steer\n"
            msg = f"{speed_cmd:.2f},{steer_cmd:.2f}\n"
            sock.sendto(msg.encode(), (ESP32_IP, ESP32_PORT))

            # --- UI DRAWING ---
            # Outer Joystick ring and center deadzone point
            cv2.circle(img, (center_x, center_y), JOYSTICK_RADIUS, (0, 0, 255), 2)
            cv2.circle(img, (center_x, center_y), 4, (0, 0, 255), -1)

            # Active Joystick Point
            point_color = (0, 255, 0) if is_pinching else (255, 0, 0)
            cv2.circle(img, (point_x, point_y), 10, point_color, -1)

            # Line & Text Feedback
            if is_pinching:
                cv2.line(img, (center_x, center_y), (point_x, point_y), (0, 255, 0), 2)
                status_text = "GRABBED"
            else:
                status_text = "RELEASED (BALANCING)"

            cv2.putText(img, f"Status: {status_text}", (20, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, point_color, 2)
            cv2.putText(img, f"Speed: {speed_cmd:.2f} | Steer: {steer_cmd:.2f}", (20, 60), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 1)

            cv2.imshow('Hand Control', img)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()