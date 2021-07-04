//
//Darbības
//
//- ieslēdz katlu un sagaida, kad sāk šņākt.
//- izslēdz katlu, palaiž OFF taimeri.
//- pēc 10s ieslēdz katlu, nofiksē OFF timeri un palaiž ON timeri.
//- kad katls sāk šņākt, izslēdz katlu, nofiksējas ON timeris un OFF timeris sāk skaitīt laiku līdz ieslēgšanai.
//- OFF timeris iztecējis, ieslēdzas katls un ON timeris sāk skaitīt laiku līdz izslēgšanai.
//- ON timeris iztecējis, izslēdzas katls un ON/OFF process aiziet ciklā.
//
//
//Pogu darbības
//
//- ieslēdz/izslēdz katlu

#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip

const int buttonPin = 4;
const int relayPin =  9;
const int ledPin =  13;

int buttonState = 0;
int previousButtonState = 0;

unsigned long timerOffMillis           = 0;
unsigned long timerOffCurrentMillis    = 0;

unsigned long timerOnMillis            = 0;
unsigned long timerOnCurrentMillis     = 0;

unsigned long globalTimeoutMillis      = 0;

const int STATE_INITIAL               = 0;
const int STATE_FIRST_TIME_ON         = 1;
const int STATE_FIRST_TIME_OFF        = 2;
const int STATE_SECOND_TIME_ON        = 3;
const int STATE_SECOND_TIME_OFF       = 4;
const int STATE_TIME_ON               = 5;
const int STATE_TIME_OFF              = 6;
const int STATE_END                   = 7;
int state                             = STATE_INITIAL;


void setup() {

  Serial.begin(9600); // open a serial connection
  Serial.println("Mains Switch\tV1.00");
  Serial.println();

  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(relayPin, LOW);

  lcd.begin(16, 2);
  lcdUpdate();
}

void loop() {

  unsigned long currentMillis = millis();
  int prevState = state;

  switch (state) {

    case STATE_INITIAL:
      break;
    case STATE_FIRST_TIME_ON:     //- ieslēdz katlu un sagaida, kad sāk šņākt.
      break;
    case STATE_FIRST_TIME_OFF:    //- palaiž OFF taimeri.

      if (timerOffCurrentMillis == 0) {
        timerOffCurrentMillis = currentMillis;
      }

      break;
    case STATE_SECOND_TIME_ON:    //- nofiksē un resetē OFF timeri un palaiž ON timeri.

      if (timerOffMillis == 0) {
        timerOffMillis = ((currentMillis - timerOffCurrentMillis) / 1000) * 1000;
        timerOffCurrentMillis = 0;
        timerOnCurrentMillis = currentMillis;
      }

      break;
    case STATE_SECOND_TIME_OFF: //- nofiksē un resetē ON timeri un palaiž OFF timeri.

      if (timerOnMillis == 0) {
        timerOnMillis = ((currentMillis - timerOnCurrentMillis) / 1000) * 1000;
        timerOnCurrentMillis = 0;
        timerOffCurrentMillis = currentMillis;
      }

      break;
    case STATE_TIME_ON: //- resetē OFF timeri un palaiž ON timeri.

      if (timerOffCurrentMillis > 0) {

        // Rewrite timer
        timerOffMillis = ((currentMillis - timerOffCurrentMillis) / 1000) * 1000;
        
        timerOffCurrentMillis = 0;
        timerOnCurrentMillis = currentMillis;
      }
      break;
    case STATE_TIME_OFF://- resetē ON timeri un palaiž OFF timeri.

      if (timerOnCurrentMillis > 0) {
 
        // Rewrite timer
        timerOnMillis = ((currentMillis - timerOnCurrentMillis) / 1000) * 1000;

        timerOnCurrentMillis = 0;
        timerOffCurrentMillis = currentMillis;
      }
      break;
    default:
      break;
  }

  // izslēdz katlu pēc ON timera iztecēšanas
  if (timerOnMillis > 0 && timerOnCurrentMillis > 0 && (currentMillis - timerOnCurrentMillis) >= timerOnMillis) {
    digitalWrite(relayPin, LOW);
    state = STATE_TIME_OFF;
  }

  // ieslēdz katlu pēc OFF timera iztecēšanas
  if (timerOffMillis > 0 && timerOffCurrentMillis > 0 && (currentMillis - timerOffCurrentMillis) >= timerOffMillis) {
    digitalWrite(relayPin, HIGH);
    state = STATE_TIME_ON;
  }

  if(globalTimeoutMillis == 0) {

    if(state == STATE_TIME_ON) {
      
      globalTimeoutMillis = 3 * 60000;
      
      Serial.println("--------Set global timeout--------");
      Serial.print("globalTimeoutMillis="); Serial.print(globalTimeoutMillis); Serial.println();
      Serial.println();
    }
  }
  else {

    if(state != STATE_END && currentMillis >= globalTimeoutMillis) {
      Serial.println("----------Global timeout--------");
      Serial.print("state="); Serial.print(state); Serial.println();
      Serial.print("currentMillis="); Serial.print(currentMillis); Serial.println();
      Serial.print("globalTimeoutMillis="); Serial.print(globalTimeoutMillis); Serial.println();

     digitalWrite(relayPin, LOW);

      // Stop timers
      timerOnCurrentMillis = 0;
      timerOffCurrentMillis = 0;
      globalTimeoutMillis = 0;

      lcd.setCursor(11, 0);
      lcd.print("     ");
      
      state = STATE_END;
    }
  }

  buttonLoop();

  lcdUpdateOnTimer(currentMillis);
  lcdUpdateOffTimer(currentMillis);
  
  lcdUpdateGlobalTimeout(currentMillis);

  if (prevState != state) {
    lcdUpdate();
  }
}

void buttonLoop() {

  buttonState = !digitalRead(buttonPin);

  digitalWrite(ledPin, buttonState);

  // Process button state
  if (buttonState == HIGH) {

    if (previousButtonState == LOW) {
      previousButtonState = HIGH;

      digitalWrite(relayPin, !digitalRead(relayPin));

      if (state == STATE_INITIAL) {
        state = STATE_FIRST_TIME_ON;
      }
      else if (state == STATE_FIRST_TIME_OFF) {
        state = STATE_SECOND_TIME_ON;
      }
      else if ((state == STATE_TIME_OFF) || (state == STATE_SECOND_TIME_OFF)) {
        state = STATE_TIME_ON;
      }
      else if (state == STATE_FIRST_TIME_ON) {
        state = STATE_FIRST_TIME_OFF;
      }
      else if (state == STATE_SECOND_TIME_ON) {
        state = STATE_SECOND_TIME_OFF;
      }
      else {
        state = STATE_TIME_OFF;
      }
    }
  }
  else {
    previousButtonState = LOW;
  }
}

void lcdUpdate() {

  switch (state) {

    case STATE_INITIAL:

      lcd.setCursor(0, 0);
      lcd.print("Ready           ");
      lcd.setCursor(0, 1);
      lcd.print("Turn ON         ");
      break;
    case STATE_FIRST_TIME_ON:     //- ieslēdz katlu un sagaida, kad sāk šņākt.

      lcd.setCursor(0, 0);
      lcd.print("First Time ON   ");
      lcd.setCursor(0, 1);
      lcd.print("Turn OFF        ");
      break;
    case STATE_FIRST_TIME_OFF:    //- palaiž OFF taimeri.

      lcd.setCursor(0, 0);
      lcd.print("First Time OFF  ");
      lcd.setCursor(0, 1);
      lcd.print("Turn ON  [S OFF]");
      break;
    case STATE_SECOND_TIME_ON:    //- nofiksē un resetē OFF timeri un palaiž ON timeri.

      lcd.setCursor(0, 0);
      lcd.print("Turned ON       ");
      lcd.setCursor(0, 1);
      lcd.print("Turn OFF  [S ON]");
      break;
    case STATE_SECOND_TIME_OFF: //- nofiksē un resetē ON timeri un palaiž OFF timeri.

      lcd.setCursor(0, 0);
      lcd.print("Turned OFF      ");
      lcd.setCursor(0, 1);
      lcd.print("Auto ON         ");
      break;
    case STATE_TIME_ON: //- resetē OFF timeri un palaiž ON timeri.

      lcd.setCursor(0, 0);
      lcd.print("Turned ON ");
      lcd.setCursor(0, 1);
      lcd.print("Auto OFF        ");
      break;
    case STATE_TIME_OFF://- resetē ON timeri un palaiž OFF timeri.

      lcd.setCursor(0, 0);
      lcd.print("Turned OFF");
      lcd.setCursor(0, 1);
      lcd.print("Auto ON         ");
      break;
    case STATE_END:

      lcd.setCursor(0, 0);
      lcd.print("Turned OFF");
      lcd.setCursor(0, 1);
      lcd.print("Done!           ");
      break;
    default:
      break;
  }
}

void lcdUpdateGlobalTimeout(unsigned long currentMillis) {

    static unsigned long lastRemainingSeconds = 0;
  
    unsigned long remainingMillis = globalTimeoutMillis - currentMillis;
    unsigned long remainingSeconds = remainingMillis / 1000;
    unsigned long minutes = remainingSeconds / 60;
    unsigned long seconds = remainingSeconds % 60;

  if(globalTimeoutMillis > 0 && (lastRemainingSeconds == 0 || lastRemainingSeconds > remainingSeconds)) {
    lastRemainingSeconds = remainingSeconds;
    lcd.setCursor(11, 0);
    lcdPrintTime(minutes, seconds);
  }
}

void lcdUpdateOnTimer(unsigned long currentMillis) {
  static unsigned long lastElapsedSeconds = 0;
  lcdUpdateTimer(currentMillis, timerOnMillis, timerOnCurrentMillis, lastElapsedSeconds);
}

void lcdUpdateOffTimer(unsigned long currentMillis) {
  static unsigned long lastElapsedSeconds = 0;
  lcdUpdateTimer(currentMillis, timerOffMillis, timerOffCurrentMillis, lastElapsedSeconds);
}

void lcdUpdateTimer(unsigned long currentMillis, unsigned long timerMillis, unsigned long timerCurrentMillis, unsigned long lastElapsedSeconds) {

  if (timerMillis > 0 && timerCurrentMillis > 0) {

    unsigned long elapsedMillis = currentMillis - timerCurrentMillis;
    unsigned long elapsedSeconds = elapsedMillis / 1000;

    if (lastElapsedSeconds != elapsedSeconds) {
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

void printTimerValues(unsigned long timerMillis, unsigned long elapsedMillis, unsigned long totalSeconds, unsigned long elapsedSeconds) {

  Serial.println();
  Serial.println("------------------- lcdUpdateTimer -------------------");
  Serial.print("timerMillis="); Serial.print(timerMillis); Serial.println();
  Serial.print("elapsedMillis="); Serial.print(elapsedMillis); Serial.println();
  Serial.print("totalSeconds="); Serial.print(totalSeconds); Serial.println();
  Serial.print("elapsedSeconds="); Serial.print(elapsedSeconds); Serial.println();
}

void lcdPrintTime(unsigned int minutes, unsigned int seconds) {

  if (minutes < 10) {
    lcd.print("0");
  }
  lcd.print(minutes);

  if(minutes > 99) {
    lcd.print("m");
  }
  else {
    lcd.print(":");

    if (seconds < 10) {
      lcd.print("0");
    }
    lcd.print(seconds);
  }
  
}
