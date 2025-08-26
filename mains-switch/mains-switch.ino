//
// Darbības
//
//- ieslēdz katlu un sagaida, kad sāk šņākt.
//- izslēdz katlu, palaiž OFF taimeri.
//- pēc 10s ieslēdz katlu, nofiksē OFF timeri un palaiž ON timeri.
//- kad katls sāk šņākt, izslēdz katlu, nofiksējas ON timeris un OFF timeris sāk skaitīt laiku līdz ieslēgšanai.
//- OFF timeris iztecējis, ieslēdzas katls un ON timeris sāk skaitīt laiku līdz izslēgšanai.
//- ON timeris iztecējis, izslēdzas katls un ON/OFF process aiziet ciklā.
//
//
// Pogu darbības
//
//- ieslēdz/izslēdz katlu (toggle current state)

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

hd44780_I2Cexp lcd;

// Pin definitions
const int buttonPin = 4;
const int relayPin = 9;
const int ledPin = 13;
const int potOnPin = A0;
const int potOffPin = A1;

// System state
bool systemRunning = false;
bool relayState = false;
int buttonState = 0;
int previousButtonState = 0;

// Timer variables
unsigned long onTimerStart = 0;
unsigned long offTimerStart = 0;
unsigned long currentOnTime = 0;
unsigned long currentOffTime = 0;

// Timer configuration
const unsigned long minTime = 10000;     // 10 seconds minimum
const unsigned long maxTime = 300000;    // 5 minutes maximum
const unsigned long baseOnTime = 60000;  // 1 minute base
const unsigned long baseOffTime = 60000; // 1 minute base

// Display update control
unsigned long lastDisplayUpdate = 0;
const unsigned long displayUpdateInterval = 100; // Update display every 100ms

void setup()
{
  Serial.begin(9600);
  Serial.println("Timer Switch - Simplified Version");

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

  // Read initial potentiometer values
  updateTimerValues();

  // Show initial screen
  displayInitialScreen();
}

void loop()
{
  unsigned long currentMillis = millis();

  // Handle button input
  handleButton();

  // Update potentiometer values periodically
  if (currentMillis - lastDisplayUpdate >= displayUpdateInterval)
  {
    updateTimerValues();
    updateDisplay(currentMillis);
    lastDisplayUpdate = currentMillis;
  }

  // Handle timer logic when system is running
  if (systemRunning)
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
    // Button pressed
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
        // Turned ON - start ON timer
        onTimerStart = millis();
        currentOnTime = getAdjustedOnTime();
        Serial.println("Relay turned ON");
      }
      else
      {
        // Turned OFF - start OFF timer
        offTimerStart = millis();
        currentOffTime = getAdjustedOffTime();
        Serial.println("Relay turned OFF");
      }
    }
  }

  previousButtonState = buttonState;
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
  int potOnValue = analogRead(potOnPin);
  int potOffValue = analogRead(potOffPin);

  // Convert potentiometer readings to multipliers (0.2 to 5.0)
  float onMultiplier = 0.2 + (potOnValue / 1023.0) * 4.8;
  float offMultiplier = 0.2 + (potOffValue / 1023.0) * 4.8;

  // Update current timer values
  currentOnTime = constrain((unsigned long)(baseOnTime * onMultiplier), minTime, maxTime);
  currentOffTime = constrain((unsigned long)(baseOffTime * offMultiplier), minTime, maxTime);
}

unsigned long getAdjustedOnTime()
{
  int potOnValue = analogRead(potOnPin);
  float onMultiplier = 0.2 + (potOnValue / 1023.0) * 4.8;
  return constrain((unsigned long)(baseOnTime * onMultiplier), minTime, maxTime);
}

unsigned long getAdjustedOffTime()
{
  int potOffValue = analogRead(potOffPin);
  float offMultiplier = 0.2 + (potOffValue / 1023.0) * 4.8;
  return constrain((unsigned long)(baseOffTime * offMultiplier), minTime, maxTime);
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
    // Show timer percentages and "Press to start"
    displayTimers();
    lcd.setCursor(0, 1);
    lcd.print("Press to start    ");
  }
  else
  {
    // Show timer percentages and current state with progress bar
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
    // Show ON state with progress bar
    lcd.print("ON ");
    displayProgressBar(currentMillis, onTimerStart, currentOnTime);
  }
  else
  {
    // Show OFF state with progress bar
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

  // Calculate progress as percentage (0-100)
  int progress = (int)((elapsed * 100) / timerDuration);

  // Display progress bar using 12 characters (3-15 positions)
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
