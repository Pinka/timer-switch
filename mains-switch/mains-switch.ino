//
// Timer Switch - Simplified Version
// Calibrated potentiometer control for 0-100 second timers
//

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

hd44780_I2Cexp lcd;

// Pin definitions
const int buttonPin = 4;
const int relayPin = 9;
const int ledPin = 13;
const int potOnPin = A0;
const int potOffPin = A0;

// System state
bool systemRunning = false;
bool relayState = false;
int buttonState = 0;
int previousButtonState = 0;

// Calibration state
bool calibrationMode = false;
int calibrationStep = 0;
const int CALIB_STEPS = 4; // ON min, ON max, OFF min, OFF max
int potOnMin = 0;
int potOnMax = 1023;
int potOffMin = 0;
int potOffMax = 1023;

// Timer variables
unsigned long onTimerStart = 0;
unsigned long offTimerStart = 0;
unsigned long currentOnTime = 0;
unsigned long currentOffTime = 0;

// Display update control
unsigned long lastDisplayUpdate = 0;
const unsigned long displayUpdateInterval = 100; // Update display every 100ms

void setup()
{
  Serial.begin(9600);
  Serial.println("Timer Switch - Calibrated Version");

  // Initialize pins
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(potOnPin, INPUT);
  pinMode(potOffPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Initialize relay to OFF
  digitalWrite(relayPin, LOW);
  relayState = false;

  // Initialize LCD
  lcd.begin(16, 2);

  // Check for calibration mode (hold button during startup)
  delay(100);
  if (!digitalRead(buttonPin))
  {
    startCalibration();
  }
  else
  {
    // Normal startup
    updateTimerValues();
    displayInitialScreen();
  }
}

void loop()
{
  unsigned long currentMillis = millis();

  // Handle button input
  handleButton();

  // Update display periodically
  if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
  {
    if (calibrationMode)
    {
      updateCalibrationDisplay();
    }
    else
    {
      updateTimerValues();
      updateDisplay(currentMillis);
    }
    lastDisplayUpdate = currentMillis;
  }

  // Handle timer logic when system is running
  if (systemRunning && !calibrationMode)
  {
    handleTimers(currentMillis);
  }
}

void handleButton()
{
  buttonState = !digitalRead(buttonPin);
  digitalWrite(ledPin, buttonState);

  if (buttonState == HIGH && previousButtonState == LOW)
  {
    if (calibrationMode)
    {
      handleCalibrationButton();
    }
    else
    {
      handleNormalButton();
    }
  }

  previousButtonState = buttonState;
}

void handleNormalButton()
{
  if (!systemRunning)
  {
    // Start the system
    systemRunning = true;
    relayState = true;
    digitalWrite(relayPin, HIGH);
    onTimerStart = millis();
    currentOnTime = getAdjustedOnTime();
    Serial.println("System started - ON cycle");
  }
  else
  {
    // Toggle relay state
    relayState = !relayState;
    digitalWrite(relayPin, relayState ? HIGH : LOW);

    if (relayState)
    {
      onTimerStart = millis();
      currentOnTime = getAdjustedOnTime();
      Serial.println("Relay turned ON");
    }
    else
    {
      offTimerStart = millis();
      currentOffTime = getAdjustedOffTime();
      Serial.println("Relay turned OFF");
    }
  }
}

void handleCalibrationButton()
{
  int potOnValue = analogRead(potOnPin);
  int potOffValue = analogRead(potOffPin);

  switch (calibrationStep)
  {
  case 0: // ON min
    potOnMin = potOnValue;
    calibrationStep++;
    break;
  case 1: // ON max
    potOnMax = potOnValue;
    calibrationStep++;
    break;
  case 2: // OFF min
    potOffMin = potOffValue;
    calibrationStep++;
    break;
  case 3: // OFF max
    potOffMax = potOffValue;
    finishCalibration();
    break;
  }
}

void startCalibration()
{
  calibrationMode = true;
  calibrationStep = 0;

  // Wait for button to be released
  while (!digitalRead(buttonPin))
  {
    delay(10);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration Mode");
  lcd.setCursor(0, 1);
  lcd.print("Turn ON pot to min");
  Serial.println("Starting calibration...");
}

void updateCalibrationDisplay()
{
  int potOnValue = analogRead(potOnPin);
  int potOffValue = analogRead(potOffPin);

  lcd.setCursor(0, 0);
  switch (calibrationStep)
  {
  case 0:
    lcd.print("ON Min: ");
    lcd.print(potOnValue);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("Press to confirm");
    break;
  case 1:
    lcd.print("ON Max: ");
    lcd.print(potOnValue);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("Press to confirm");
    break;
  case 2:
    lcd.print("OFF Min: ");
    lcd.print(potOffValue);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    lcd.print("Press to confirm");
    break;
  case 3:
    lcd.print("OFF Max: ");
    lcd.print(potOffValue);
    lcd.print("   ");
    lcd.setCursor(0, 1);
    lcd.print("Press to confirm");
    break;
  }
}

void finishCalibration()
{
  calibrationMode = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration Done!");
  lcd.setCursor(0, 1);
  lcd.print("ON:");
  lcd.print(potOnMin);
  lcd.print("-");
  lcd.print(potOnMax);
  lcd.print(" OFF:");
  lcd.print(potOffMin);
  lcd.print("-");
  lcd.print(potOffMax);
  delay(2000);

  updateTimerValues();
  displayInitialScreen();
  Serial.println("Calibration completed");
}

void handleTimers(unsigned long currentMillis)
{
  if (relayState)
  {
    // Relay is ON - check if ON timer expired
    if (currentMillis - onTimerStart >= currentOnTime)
    {
      relayState = false;
      digitalWrite(relayPin, LOW);
      offTimerStart = currentMillis;
      currentOffTime = getAdjustedOffTime();
      Serial.println("ON timer expired - turning OFF");
    }
  }
  else
  {
    // Relay is OFF - check if OFF timer expired
    if (currentMillis - offTimerStart >= currentOffTime)
    {
      relayState = true;
      digitalWrite(relayPin, HIGH);
      onTimerStart = currentMillis;
      currentOnTime = getAdjustedOnTime();
      Serial.println("OFF timer expired - turning ON");
    }
  }
}

void updateTimerValues()
{
  currentOnTime = getAdjustedOnTime();
  currentOffTime = getAdjustedOffTime();
}

unsigned long getAdjustedOnTime()
{
  int potOnValue = analogRead(potOnPin);
  int mappedValue;

  // Handle inverted ranges (max < min)
  if (potOnMax < potOnMin)
  {
    mappedValue = map(potOnValue, potOnMax, potOnMin, 0, 100);
  }
  else
  {
    mappedValue = map(potOnValue, potOnMin, potOnMax, 0, 100);
  }

  mappedValue = constrain(mappedValue, 0, 100);
  return mappedValue * 1000; // Convert to milliseconds
}

unsigned long getAdjustedOffTime()
{
  int potOffValue = analogRead(potOffPin);
  int mappedValue;

  // Handle inverted ranges (max < min)
  if (potOffMax < potOffMin)
  {
    mappedValue = map(potOffValue, potOffMax, potOffMin, 0, 100);
  }
  else
  {
    mappedValue = map(potOffValue, potOffMin, potOffMax, 0, 100);
  }

  mappedValue = constrain(mappedValue, 0, 100);
  return mappedValue * 1000; // Convert to milliseconds
}

void displayInitialScreen()
{
  lcd.clear();
  displayTimers();
  lcd.setCursor(0, 1);
  lcd.print("Press to start");
}

void updateDisplay(unsigned long currentMillis)
{
  if (!systemRunning)
  {
    displayTimers();
    lcd.setCursor(0, 1);
    lcd.print("Press to start    ");
  }
  else
  {
    displayTimers();
    displayRunningState(currentMillis);
  }
}

void displayTimers()
{
  int onSeconds = (int)(currentOnTime / 1000);
  int offSeconds = (int)(currentOffTime / 1000);

  lcd.setCursor(0, 0);
  lcd.print("ON:");
  if (onSeconds < 100)
    lcd.print(" ");
  lcd.print(onSeconds);
  lcd.print("s");

  lcd.setCursor(8, 0);
  lcd.print("OFF:");
  if (offSeconds < 100)
    lcd.print(" ");
  lcd.print(offSeconds);
  lcd.print("s");
}

void displayRunningState(unsigned long currentMillis)
{
  lcd.setCursor(0, 1);

  if (relayState)
  {
    lcd.print("ON ");
    displayProgressBar(currentMillis, onTimerStart, currentOnTime);
  }
  else
  {
    lcd.print("OFF");
    displayProgressBar(currentMillis, offTimerStart, currentOffTime);
  }
}

void displayProgressBar(unsigned long currentMillis, unsigned long timerStart, unsigned long timerDuration)
{
  if (timerStart == 0)
    return;

  unsigned long elapsed = currentMillis - timerStart;
  if (elapsed > timerDuration)
    elapsed = timerDuration;

  int progress = (int)((elapsed * 100) / timerDuration);

  lcd.setCursor(3, 1);
  int barLength = (progress * 12) / 100;

  for (int i = 0; i < 12; i++)
  {
    if (i < barLength)
    {
      lcd.write(255); // Full block character
    }
    else
    {
      lcd.print(" ");
    }
  }
}
