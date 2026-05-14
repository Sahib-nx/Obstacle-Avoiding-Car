// ─── Pin Configuration ───────────────────────────────────────────────────────
const int trigPin = 9;
const int echoPin = 10;
const int IN1 = 5;   // Right motor forward
const int IN2 = 6;   // Right motor backward
const int IN3 = 7;   // Left motor forward
const int IN4 = 8;   // Left motor backward
const int led = 3;
const int buzzer = 4;

// ─── Tuning Constants ────────────────────────────────────────────────────────
const int OBSTACLE_DIST     = 30;   // cm — obstacle detection threshold
const int CLEAR_DIST        = 25;   // cm — minimum "safe" distance after turning
const int REVERSE_TIME      = 400;  // ms — how long to reverse
const int TURN_90_TIME      = 550;  // ms — tune this to get ~90° on your surface
const int TURN_45_TIME      = 300;  // ms — tune this to get ~45°
const int POST_MOVE_PAUSE   = 150;  // ms — brief settle pause after each action

// ─── Distance Measurement ────────────────────────────────────────────────────
long getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30 ms timeout (~5 m max)
  if (duration == 0) return 999;                 // no echo → treat as open path

  long distance = duration * 0.034 / 2;
  return distance;
}

// ─── Motor Primitives ────────────────────────────────────────────────────────

/*
 * Right motor  → IN1 HIGH / IN2 LOW  = forward
 *                IN1 LOW  / IN2 HIGH = backward
 * Left  motor  → IN3 HIGH / IN4 LOW  = forward
 *                IN3 LOW  / IN4 HIGH = backward
 */

void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // right forward
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // left forward
}

void moveBackward() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);  // right backward
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);  // left backward
}

void stopCar() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);
}

/*
 * Pivot RIGHT (clockwise when viewed from above):
 *   Left  wheel → forward
 *   Right wheel → stopped (zero-radius pivot around right wheel)
 *
 * This is safer than counter-rotating for carpet / uneven ground.
 * For a tighter spin, replace right-wheel STOP with backward — see comment.
 */
void pivotRight(int duration_ms) {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);   // right wheel: stop
  // To counter-rotate instead: IN1=LOW, IN2=HIGH
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // left wheel: forward
  delay(duration_ms);
  stopCar();
}

/*
 * Pivot LEFT (counter-clockwise):
 *   Right wheel → forward
 *   Left  wheel → stopped
 */
void pivotLeft(int duration_ms) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // right wheel: forward
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);   // left wheel: stop
  delay(duration_ms);
  stopCar();
}

// ─── Alert Helpers ───────────────────────────────────────────────────────────
void alertOn() {
  digitalWrite(led, HIGH);
  tone(buzzer, 1000);
}

void alertOff() {
  digitalWrite(led, LOW);
  noTone(buzzer);
}

// ─── Setup ───────────────────────────────────────────────────────────────────
void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(led, OUTPUT);
  pinMode(buzzer, OUTPUT);
  Serial.begin(9600); // optional — useful for tuning distances
}

// ─── Main Avoidance Logic ────────────────────────────────────────────────────
void handleObstacle() {
  // ── 1. Alert & stop ───────────────────────────────────────────────────────
  alertOn();
  stopCar();
  delay(POST_MOVE_PAUSE);

  // ── 2. Reverse slightly to gain clearance for turning ────────────────────
  moveBackward();
  delay(REVERSE_TIME);
  stopCar();
  delay(POST_MOVE_PAUSE);
  alertOff();

  // ── 3. Try RIGHT first ────────────────────────────────────────────────────
  pivotRight(TURN_90_TIME);
  delay(POST_MOVE_PAUSE);

  long rightDist = getDistance();
  Serial.print("Right dist: "); Serial.println(rightDist);

  if (rightDist >= CLEAR_DIST) {
    // Right is clear — continue forward in this new direction
    moveForward();
    return;
  }

  // ── 4. Right blocked — pivot back to center then try LEFT ────────────────
  //    Pivot left ~180° total (90° back to center + 90° past to left side)
  pivotLeft(TURN_90_TIME * 2);
  delay(POST_MOVE_PAUSE);

  long leftDist = getDistance();
  Serial.print("Left dist: "); Serial.println(leftDist);

  if (leftDist >= CLEAR_DIST) {
    // Left is clear — continue forward in this new direction
    moveForward();
    return;
  }

  // ── 5. Both sides blocked — reverse more and retry next loop iteration ───
  alertOn();
  moveBackward();
  delay(REVERSE_TIME);
  stopCar();
  delay(300);
  alertOff();

  // Pivot back roughly to original heading before retrying
  pivotRight(TURN_90_TIME);
  delay(POST_MOVE_PAUSE);
}

// ─── Loop ────────────────────────────────────────────────────────────────────
void loop() {
  long d = getDistance();
  Serial.print("Front dist: "); Serial.println(d);

  if (d >= OBSTACLE_DIST) {
    moveForward();
    alertOff();
  } else {
    handleObstacle();
  }

  delay(100);
}