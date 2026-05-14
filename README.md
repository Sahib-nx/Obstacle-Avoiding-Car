# 🤖 Arduino Obstacle-Avoiding Robot Car

An Arduino UNO-based two-wheeled robot that autonomously navigates around obstacles using an HC-SR04 ultrasonic sensor and differential drive steering , **no servo motor required**.

---

## 📌 Project Overview

This robot was built and programmed to demonstrate real-world autonomous navigation using affordable, widely available components. It started as a simple forward-or-stop robot and was upgraded to full obstacle avoidance logic with directional decision-making , checking right, then left, and falling back to reverse-and-retry if both paths are blocked.

---

## 🧠 How It Works

### The HC-SR04 Ultrasonic Sensor
The **HC-SR04** is an ultrasonic distance sensor that works by emitting a short burst of high-frequency sound (40 kHz) from its **Trig** pin. When that sound wave hits an object and bounces back, the **Echo** pin goes HIGH for a duration proportional to the distance. The microcontroller measures that pulse width and converts it to centimeters using the formula:

```
distance (cm) = pulse duration (µs) × 0.034 / 2
```

- **Operating voltage:** 5V
- **Detection range:** ~2 cm to 400 cm
- **Accuracy:** ±3 mm
- **Angle:** ~15° cone

### Differential Drive Steering
Instead of a servo motor to rotate the sensor, this robot uses **differential drive** , controlling each wheel independently to steer:

| Action | Left Wheel | Right Wheel |
|---|---|---|
| Move Forward | Forward | Forward |
| Move Backward | Backward | Backward |
| Pivot Right | Forward | Stopped |
| Pivot Left | Stopped | Forward |
| Stop | Stopped | Stopped |

By driving one wheel and stopping the other, the robot pivots in place by a fixed angle controlled through **timed delays** (tunable constants in the code).

---

## 🔧 Hardware Components

| Component | Purpose |
|---|---|
| Arduino UNO | Main microcontroller |
| HC-SR04 Ultrasonic Sensor | Measures distance to obstacles |
| L298N Motor Driver Module | Controls 2 DC motors (direction + power) |
| 2× DC Geared Motors | Drive the left and right wheels |
| LED | Visual alert when obstacle is detected |
| Buzzer (passive) | Audio alert when obstacle is detected |
| 2x 18650 Battery Pack + Holder | Power supply for motors |
| Jumper Wires + Wooden Chassis | Physical assembly |

---

## 📐 Pin Configuration

| Pin | Component | Role |
|---|---|---|
| 9 | HC-SR04 Trig | Send ultrasonic pulse |
| 10 | HC-SR04 Echo | Receive echo |
| 5 | L298N IN1 | Right motor forward |
| 6 | L298N IN2 | Right motor backward |
| 7 | L298N IN3 | Left motor forward |
| 8 | L298N IN4 | Left motor backward |
| 3 | LED | Obstacle alert indicator |
| 4 | Buzzer | Obstacle alert tone |

---

## ⚙️ Obstacle Avoidance Logic

The robot follows this decision tree every 100 ms:

```
┌─────────────────────────────────────┐
│     Measure front distance          │
└────────────────┬────────────────────┘
                 │
        ┌────────▼─────────┐
        │  Distance ≥ 30cm? │
        └────┬─────────┬────┘
           YES         NO
            │           │
     Move Forward    Stop + Alert
                        │
                    Reverse 400ms
                        │
                  Pivot Right 90°
                        │
               ┌────────▼─────────┐
               │  Right path clear? │
               └────┬─────────┬────┘
                  YES         NO
                   │           │
           Move Forward   Pivot Left 180°
                               │
                    ┌──────────▼──────────┐
                    │  Left path clear?    │
                    └────┬─────────┬───────┘
                       YES         NO
                        │           │
                Move Forward    Reverse more
                                    + Retry
```

### Tunable Constants

These values in the code can be adjusted to match your specific motors and floor surface:

```cpp
const int OBSTACLE_DIST   = 30;   // cm  — detection threshold
const int CLEAR_DIST      = 25;   // cm  — "safe" path threshold after turning
const int REVERSE_TIME    = 400;  // ms  — duration of backward movement
const int TURN_90_TIME    = 550;  // ms  — time to rotate ~90° (tune this!)
const int POST_MOVE_PAUSE = 150;  // ms  — settle pause between actions
```

> **Calibration tip:** Place the robot on your actual floor surface, run `pivotRight(TURN_90_TIME)` in isolation, measure the angle visually, and adjust `TURN_90_TIME` until you get a clean 90° rotation. Carpet requires more time than tile or hardwood.

---

## 🔄 Movement Functions

### `moveForward()`
Both wheels spin forward at full speed.

### `moveBackward()`
Both wheels spin backward — used to gain clearance before turning.

### `stopCar()`
All motor pins set LOW — immediate brake.

### `pivotRight(int duration_ms)`
Left wheel spins forward, right wheel is braked. The robot rotates clockwise (when viewed from above) for the given duration, then stops.

### `pivotLeft(int duration_ms)`
Right wheel spins forward, left wheel is braked. The robot rotates counter-clockwise for the given duration, then stops.

### `getDistance()`
Triggers the HC-SR04, measures the echo pulse with a 30 ms timeout (prevents blocking if no obstacle is detected), and returns distance in centimeters. Returns `999` if no echo is received.

### `alertOn()` / `alertOff()`
Turns the LED and buzzer on or off to indicate obstacle detection status.

---

## 🚀 Getting Started

### Prerequisites
- [Arduino IDE](https://www.arduino.cc/en/software) (1.8.x or 2.x)
- Arduino UNO board
- All hardware components listed above

### Upload Steps
1. Clone or download this repository
2. Open `ObstacleAvoidingCar.ino` in Arduino IDE
3. Select **Board:** Arduino UNO
4. Select the correct **Port**
5. Click **Upload**
6. Open Serial Monitor at **9600 baud** to view live distance readings (useful for tuning)

---

## 🛠️ Improvements from Original Version

| Feature | Original Code | Upgraded Code |
|---|---|---|
| Obstacle response | Stop only | Stop → Reverse → Turn → Check |
| Direction checking | None | Right first, then Left |
| Sensor timeout | None (blocks ~1 sec) | 30 ms timeout, returns 999 |
| Both-sides-blocked | N/A | Reverse + retry logic |
| Alert control | Permanent when stopped | Contextual on/off per action |
| Tuning | Hardcoded values | Named constants at top of file |
| Debug output | None | Serial Monitor distance logging |

---

## ⚠️ Known Limitations

- Turning angle is time-based, so it can drift on different surfaces , calibrate `TURN_90_TIME` for your floor.
- The single front-facing sensor cannot detect obstacles to the side while moving forward.
- Very narrow gaps or thin obstacles (like chair legs) may not reflect enough ultrasonic signal to be detected reliably.
- Battery voltage drop over time can affect motor speed and therefore turning accuracy.

---

## 📜 License

This project is open source and free to use for educational purposes.

---

## 🙋 Author

Built as part of a Software Engineering hardware project. Contributions and improvements welcome.
