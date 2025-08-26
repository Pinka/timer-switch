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
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip

const int buttonPin = 4;
const int relayPin = 9;
const int ledPin = 13;
const int potOnPin = A0;  // Potentiometer for ON time connected to analog pin A0
const int potOffPin = A1; // Potentiometer for OFF time connected to analog pin A1

int buttonState = 0;
int previousButtonState = 0;

unsigned long timerOffMillis = 0;
unsigned long timerOffCurrentMillis = 0;

unsigned long timerOnMillis = 0;
unsigned long timerOnCurrentMillis = 0;

unsigned long globalTimeoutMillis = 0;

// Potentiometer adjustment variables
unsigned long baseOnTime = 60000;  // Base ON time: 1 minute (60 seconds)
unsigned long baseOffTime = 60000; // Base OFF time: 1 minute (60 seconds)
unsigned long minTime = 10000;     // Minimum time: 10 seconds
unsigned long maxTime = 300000;    // Maximum time: 5 minutes (300 seconds)
float onTimeMultiplier = 1.0;      // Multiplier for ON time adjustment
float offTimeMultiplier = 1.0;     // Multiplier for OFF time adjustment

const int STATE_INITIAL = 0;
const int STATE_FIRST_TIME_ON = 1;
const int STATE_FIRST_TIME_OFF = 2;
const int STATE_SECOND_TIME_ON = 3;
const int STATE_SECOND_TIME_OFF = 4;
const int STATE_TIME_ON = 5;
const int STATE_TIME_OFF = 6;
const int STATE_END = 7;
int state = STATE_INITIAL;

// System state tracking
bool systemRunning = false;
bool relayState = false;

void setup()
{
  Serial.begin(9600); // open a serial connection
  Serial.println("Mains Switch\tV2.00");
  Serial.println("Dual Potentiometer Control");
  Serial.println();

  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(potOnPin, INPUT);  // Set ON potentiometer pin as input
  pinMode(potOffPin, INPUT); // Set OFF potentiometer pin as input

  digitalWrite(relayPin, LOW);
  relayState = false;

  lcd.begin(16, 2);
  lcdUpdate();

  // Read initial potentiometer values
  readPotentiometerValues();
}

void loop()
{
  unsigned long currentMillis = millis();
  int prevState = state;

  // Handle automatic cycling only when system is running
  if (systemRunning)
  {
    switch (state)
    {
    case STATE_INITIAL:
      break;
    case STATE_FIRST_TIME_ON: //- ieslēdz katlu un sagaida, kad sāk šņākt.
      break;
    case STATE_FIRST_TIME_OFF: //- palaiž OFF taimeri.
      if (timerOffCurrentMillis == 0)
      {
        timerOffCurrentMillis = currentMillis;
      }
      break;
    case STATE_SECOND_TIME_ON: //- nofiksē un resetē OFF timeri un palaiž ON timeri.
      if (timerOffMillis == 0)
      {
        // Use adjusted OFF time based on potentiometer
        timerOffMillis = getAdjustedOffTime();
        timerOffCurrentMillis = 0;
        timerOnCurrentMillis = currentMillis;
      }
      break;
    case STATE_SECOND_TIME_OFF: //- nofiksē un resetē ON timeri un palaiž OFF timeri.
      if (timerOnMillis == 0)
      {
        // Use adjusted ON time based on potentiometer
        timerOnMillis = getAdjustedOnTime();
        timerOnCurrentMillis = 0;
        timerOffCurrentMillis = currentMillis;
      }
      break;
    case STATE_TIME_ON: //- resetē OFF timeri un palaiž ON timeri.
      if (timerOffCurrentMillis > 0)
      {
        // Use adjusted OFF time based on potentiometer
        timerOffMillis = getAdjustedOffTime();
        timerOffCurrentMillis = 0;
        timerOnCurrentMillis = currentMillis;
      }
      break;
    case STATE_TIME_OFF: //- resetē ON timeri un palaiž OFF timeri.
      if (timerOnCurrentMillis > 0)
      {
        // Use adjusted ON time based on potentiometer
        timerOnMillis = getAdjustedOnTime();
        timerOnCurrentMillis = 0;
        timerOffCurrentMillis = currentMillis;
      }
      break;
    default:
      break;
    }

    // izslēdz katlu pēc ON timera iztecēšanas
    if (timerOnMillis > 0 && timerOnCurrentMillis > 0 && (currentMillis - timerOnCurrentMillis) >= timerOnMillis)
    {
      digitalWrite(relayPin, LOW);
      relayState = false;
      state = STATE_TIME_OFF;
    }

    // ieslēdz katlu pēc OFF timera iztecēšanas
    if (timerOffMillis > 0 && timerOffCurrentMillis > 0 && (currentMillis - timerOffCurrentMillis) >= timerOffMillis)
    {
      digitalWrite(relayPin, HIGH);
      relayState = true;
      state = STATE_TIME_ON;
    }

    if (globalTimeoutMillis == 0)
    {
      if (state == STATE_TIME_ON)
      {
        globalTimeoutMillis = 120 * 60000;
        Serial.println("--------Set global timeout--------");
        Serial.print("globalTimeoutMillis=");
        Serial.print(globalTimeoutMillis);
        Serial.println();
        Serial.println();
      }
    }
    else
    {
      if (state != STATE_END && currentMillis >= globalTimeoutMillis)
      {
        Serial.println("----------Global timeout--------");
        Serial.print("state=");
        Serial.print(state);
        Serial.println();
        Serial.print("currentMillis=");
        Serial.print(currentMillis);
        Serial.println();
        Serial.print("globalTimeoutMillis=");
        Serial.print(globalTimeoutMillis);
        Serial.println();

        digitalWrite(relayPin, LOW);
        relayState = false;

        // Stop timers
        timerOnCurrentMillis = 0;
        timerOffCurrentMillis = 0;
        globalTimeoutMillis = 0;
        systemRunning = false;

        lcd.setCursor(11, 0);
        lcd.print("     ");

        state = STATE_END;
      }
    }
  }

  buttonLoop();

  // Read potentiometer values periodically (every 100ms)
  static unsigned long lastPotRead = 0;
  if (currentMillis - lastPotRead >= 100)
  {
    readPotentiometerValues();
    lastPotRead = currentMillis;
  }

  lcdUpdateOnTimer(currentMillis);
  lcdUpdateOffTimer(currentMillis);
  lcdUpdateGlobalTimeout(currentMillis);
  lcdUpdatePotentiometer(); // Update potentiometer display

  if (prevState != state)
  {
    lcdUpdate();
  }
}

void buttonLoop()
{
  buttonState = !digitalRead(buttonPin);
  digitalWrite(ledPin, buttonState);

  // Process button state
  if (buttonState == HIGH)
  {
    if (previousButtonState == LOW)
    {
      previousButtonState = HIGH;

      if (!systemRunning)
      {
        // Start the system
        systemRunning = true;
        digitalWrite(relayPin, HIGH);
        relayState = true;
        state = STATE_FIRST_TIME_ON;
        Serial.println("System started - First ON cycle");
      }
      else
      {
        // Toggle relay state
        relayState = !relayState;
        digitalWrite(relayPin, relayState ? HIGH : LOW);

        // Update state based on relay state
        if (relayState)
        {
          if (state == STATE_TIME_OFF || state == STATE_SECOND_TIME_OFF)
          {
            state = STATE_TIME_ON;
          }
          else if (state == STATE_FIRST_TIME_OFF)
          {
            state = STATE_SECOND_TIME_ON;
          }
          else
          {
            state = STATE_FIRST_TIME_ON;
          }
        }
        else
        {
          if (state == STATE_TIME_ON || state == STATE_SECOND_TIME_ON)
          {
            state = STATE_TIME_OFF;
          }
          else if (state == STATE_FIRST_TIME_ON)
          {
            state = STATE_FIRST_TIME_OFF;
          }
          else
          {
            state = STATE_SECOND_TIME_OFF;
          }
        }

        Serial.print("Relay toggled to: ");
        Serial.println(relayState ? "ON" : "OFF");
      }
    }
  }
  else
  {
    previousButtonState = LOW;
  }
}

void lcdUpdate()
{
  switch (state)
  {
  case STATE_INITIAL:
    lcd.setCursor(0, 0);
    lcd.print("Ready           ");
    lcd.setCursor(0, 1);
    lcd.print("Press to start  ");
    break;
  case STATE_FIRST_TIME_ON: //- ieslēdz katlu un sagaida, kad sāk šņākt.
    lcd.setCursor(0, 0);
    lcd.print("First Time ON   ");
    lcd.setCursor(0, 1);
    lcd.print("Press to OFF    ");
    break;
  case STATE_FIRST_TIME_OFF: //- palaiž OFF taimeri.
    lcd.setCursor(0, 0);
    lcd.print("First Time OFF  ");
    lcd.setCursor(0, 1);
    lcd.print("Press to ON     ");
    break;
  case STATE_SECOND_TIME_ON: //- nofiksē un resetē OFF timeri un palaiž ON timeri.
    lcd.setCursor(0, 0);
    lcd.print("Turned ON       ");
    lcd.setCursor(0, 1);
    lcd.print("Press to OFF    ");
    break;
  case STATE_SECOND_TIME_OFF: //- nofiksē un resetē ON timeri un palaiž OFF timeri.
    lcd.setCursor(0, 0);
    lcd.print("Turned OFF      ");
    lcd.setCursor(0, 1);
    lcd.print("Press to ON     ");
    break;
  case STATE_TIME_ON: //- resetē OFF timeri un palaiž ON timeri.
    lcd.setCursor(0, 0);
    lcd.print("Turned ON ");
    lcd.setCursor(0, 1);
    lcd.print("Press to OFF    ");
    break;
  case STATE_TIME_OFF: //- resetē ON timeri un palaiž OFF timeri.
    lcd.setCursor(0, 0);
    lcd.print("Turned OFF");
    lcd.setCursor(0, 1);
    lcd.print("Press to ON     ");
    break;
  case STATE_END:
    lcd.setCursor(0, 0);
    lcd.print("System stopped  ");
    lcd.setCursor(0, 1);
    lcd.print("Press to restart");
    break;
  default:
    break;
  }
}

void lcdUpdateGlobalTimeout(unsigned long currentMillis)
{

  static unsigned long lastRemainingSeconds = 0;

  unsigned long remainingMillis = globalTimeoutMillis - currentMillis;
  unsigned long remainingSeconds = remainingMillis / 1000;
  unsigned long minutes = remainingSeconds / 60;
  unsigned long seconds = remainingSeconds % 60;

  if (globalTimeoutMillis > 0 && (lastRemainingSeconds == 0 || lastRemainingSeconds > remainingSeconds))
  {
    lastRemainingSeconds = remainingSeconds;
    lcd.setCursor(11, 0);
    lcdPrintTime(minutes, seconds);
  }
}

void lcdUpdateOnTimer(unsigned long currentMillis)
{
  static unsigned long lastElapsedSeconds = 0;
  lcdUpdateTimer(currentMillis, timerOnMillis, timerOnCurrentMillis, lastElapsedSeconds);
}

void lcdUpdateOffTimer(unsigned long currentMillis)
{
  static unsigned long lastElapsedSeconds = 0;
  lcdUpdateTimer(currentMillis, timerOffMillis, timerOffCurrentMillis, lastElapsedSeconds);
}

void lcdUpdateTimer(unsigned long currentMillis, unsigned long timerMillis, unsigned long timerCurrentMillis, unsigned long lastElapsedSeconds)
{

  if (timerMillis > 0 && timerCurrentMillis > 0)
  {

    unsigned long elapsedMillis = currentMillis - timerCurrentMillis;
    unsigned long elapsedSeconds = elapsedMillis / 1000;

    if (lastElapsedSeconds != elapsedSeconds)
    {
      lastElapsedSeconds = elapsedSeconds;

      unsigned long totalSeconds = timerMillis / 1000;
      unsigned long remainingSeconds = totalSeconds - elapsedSeconds;
      unsigned long minutes = remainingSeconds / 60;
      unsigned long seconds = remainingSeconds % 60;

      lcd.setCursor(11, 1);
      lcdPrintTime(minutes, seconds);
      printTimerValues(timerMillis, elapsedMillis, totalSeconds, elapsedSeconds);
    }
  }
}

void printTimerValues(unsigned long timerMillis, unsigned long elapsedMillis, unsigned long totalSeconds, unsigned long elapsedSeconds)
{

  Serial.println();
  Serial.println("------------------- lcdUpdateTimer -------------------");
  Serial.print("timerMillis=");
  Serial.print(timerMillis);
  Serial.println();
  Serial.print("elapsedMillis=");
  Serial.print(elapsedMillis);
  Serial.println();
  Serial.print("totalSeconds=");
  Serial.print(totalSeconds);
  Serial.println();
  Serial.print("elapsedSeconds=");
  Serial.print(elapsedSeconds);
  Serial.println();
}

void lcdPrintTime(unsigned int minutes, unsigned int seconds)
{

  if (minutes < 10)
  {
    lcd.print("0");
  }
  lcd.print(minutes);

  if (minutes > 99)
  {
    lcd.print("m");
  }
  else
  {
    lcd.print(":");

    if (seconds < 10)
    {
      lcd.print("0");
    }
    lcd.print(seconds);
  }
}

// Read potentiometer values and calculate multipliers
void readPotentiometerValues()
{
  int potOnValue = analogRead(potOnPin);   // Read ON time potentiometer (0-1023)
  int potOffValue = analogRead(potOffPin); // Read OFF time potentiometer (0-1023)

  // Convert potentiometer readings to multipliers (0.2 to 5.0)
  // This gives a range from 20% to 500% of base time
  onTimeMultiplier = 0.2 + (potOnValue / 1023.0) * 4.8;
  offTimeMultiplier = 0.2 + (potOffValue / 1023.0) * 4.8;

  // Debug output
  // Serial.print("ON Pot: ");
  // Serial.print(potOnValue);
  // Serial.print(", OFF Pot: ");
  // Serial.print(potOffValue);
  // Serial.print(", ON Mult: ");
  // Serial.print(onTimeMultiplier, 2);
  // Serial.print(", OFF Mult: ");
  // Serial.print(offTimeMultiplier, 2);
  // Serial.println();
}

// Calculate adjusted timer values based on potentiometer
unsigned long getAdjustedOnTime()
{
  unsigned long adjustedTime = (unsigned long)(baseOnTime * onTimeMultiplier);

  // Constrain to min/max values
  if (adjustedTime < minTime)
    adjustedTime = minTime;
  if (adjustedTime > maxTime)
    adjustedTime = maxTime;

  return adjustedTime;
}

unsigned long getAdjustedOffTime()
{
  unsigned long adjustedTime = (unsigned long)(baseOffTime * offTimeMultiplier);

  // Constrain to min/max values
  if (adjustedTime < minTime)
    adjustedTime = minTime;
  if (adjustedTime > maxTime)
    adjustedTime = maxTime;

  return adjustedTime;
}

// Update potentiometer adjustment display
void lcdUpdatePotentiometer()
{
  static int lastDisplayedOnMultiplier = 0;
  static int lastDisplayedOffMultiplier = 0;
  int currentOnMultiplier = (int)(onTimeMultiplier * 100);
  int currentOffMultiplier = (int)(offTimeMultiplier * 100);

  // Only update if the values have changed
  if (lastDisplayedOnMultiplier != currentOnMultiplier || lastDisplayedOffMultiplier != currentOffMultiplier)
  {
    lastDisplayedOnMultiplier = currentOnMultiplier;
    lastDisplayedOffMultiplier = currentOffMultiplier;

    // Show adjustment percentages when in initial state
    if (state == STATE_INITIAL)
    {
      lcd.setCursor(0, 0);
      lcd.print("ON:");
      if (currentOnMultiplier < 100)
      {
        lcd.print(" ");
      }
      lcd.print(currentOnMultiplier);
      lcd.print("%");

      lcd.setCursor(8, 0);
      lcd.print("OFF:");
      if (currentOffMultiplier < 100)
      {
        lcd.print(" ");
      }
      lcd.print(currentOffMultiplier);
      lcd.print("%");
    }
  }
}
