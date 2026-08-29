#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <SoftwareSerial.h>

// ================= LCD =================
hd44780_I2Cexp lcd(0x27);

// ================= BLUETOOTH =================
SoftwareSerial BT(8, 9);

// ================= SENSOR =================
const int hallPin = 2;
const int numMagnets = 3;
float radius = 0.33;   // ✅ UPDATED (wheel radius)

// ================= USER PARAM =================
float weight = 76.5;   // DEFAULT WEIGHT

// ================= VARIABLES =================
volatile unsigned long lastPulseTime = 0;
volatile unsigned long deltaT = 0;
volatile bool newPulse = false;

float totalDistance = 0.0;
float speed = 0.0;

// Filtering
const int windowSize = 5;
float speedBuffer[windowSize] = {0};
int bufferIndex = 0;
int validCount = 0;

// Timing
unsigned long lastDisplayTime = 0;
unsigned long lastCalorieTime = 0;

const unsigned long displayInterval = 300;
const unsigned long calorieInterval = 1000;

// Debounce
const unsigned long minInterval = 800;

// Calories
float calories = 0.0;

// ================= INTERRUPT =================
void handlePulse() {
  unsigned long now = micros();

  if (lastPulseTime > 0) {
    unsigned long interval = now - lastPulseTime;

    if (interval > minInterval) {
      deltaT = interval;
      newPulse = true;
    }
  }

  lastPulseTime = now;
}

// ================= SETUP =================
void setup() {
  pinMode(hallPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(hallPin), handlePulse, FALLING);

  lcd.begin(16, 2);
  lcd.backlight();

  BT.begin(9600);

  lcd.setCursor(0, 0);
  lcd.print(" Have A Nice Day ");
  lcd.setCursor(0, 1);
  lcd.print("     Buddy :)  ");
  delay(3000);
  lcd.clear();
}

// ================= LOOP =================
void loop() {

  static unsigned long lastDeltaT = 0;

  unsigned long localDeltaT;
  bool pulseDetected;

  noInterrupts();
  localDeltaT = deltaT;
  pulseDetected = newPulse;
  newPulse = false;
  interrupts();

  // ===== SPEED + DISTANCE =====
  if (pulseDetected && localDeltaT > 0) {

    float timeSec = localDeltaT / 1e6;
    float distancePerPulse = (2 * PI * radius) / numMagnets;

    float newSpeed = distancePerPulse / timeSec;

    if (newSpeed > 0.05 && newSpeed < 30.0) {

      totalDistance += distancePerPulse;

      speedBuffer[bufferIndex] = newSpeed;
      bufferIndex = (bufferIndex + 1) % windowSize;

      if (validCount < windowSize) validCount++;

      float sum = 0;
      for (int i = 0; i < validCount; i++) {
        sum += speedBuffer[i];
      }

      speed = sum / validCount;
      lastDeltaT = localDeltaT;
    }
  }

  // ===== ZERO SPEED =====
  if (lastDeltaT > 0) {
    if ((micros() - lastPulseTime) > 2 * lastDeltaT) {
      speed = 0.0;
    }
  }

  float speedKmh = speed * 3.6;
  float distanceKm = totalDistance / 1000.0;

  // ===== MET =====
  float MET = 0.0;

  if (speedKmh > 1 && speedKmh <= 10) MET = 4.0;
  else if (speedKmh <= 15) MET = 6.0;
  else if (speedKmh <= 20) MET = 8.0;
  else if (speedKmh <= 25) MET = 10.0;
  else if (speedKmh > 25) MET = 12.0;

  // ===== CALORIES =====
  if (millis() - lastCalorieTime > calorieInterval) {

    if (speedKmh > 1.0) {
      float kcal_per_sec = (MET * weight * 3.5) / 200.0 / 60.0;
      calories += kcal_per_sec;
    }

    lastCalorieTime = millis();
  }

  // ===== LCD =====
  if (millis() - lastDisplayTime > displayInterval) {

    lcd.setCursor(0, 0);
    lcd.print("Sp: ");
    lcd.print(speedKmh, 1);
    lcd.print(" Dis:");
    lcd.print(distanceKm, 2);

    lcd.setCursor(0, 1);
    lcd.print("Cal: ");
    lcd.print(calories, 1);
    lcd.print("      ");

    lastDisplayTime = millis();
  }

  // ===== BLUETOOTH =====
  BT.print(speedKmh, 2);
  BT.print(",");
  BT.print(distanceKm, 3);
  BT.print(",");
  BT.print(calories, 2);
  BT.print(";");

  delay(100);
}