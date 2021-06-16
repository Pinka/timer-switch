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

const int buttonPin = 4;
const int relayPin =  9;
const int ledPin =  13;

int buttonState = 0;
int previousButtonState = 0;

unsigned long timerOffMillis           = 0;
unsigned long timerOffCurrentMillis    = 0;

unsigned long timerOnMillis            = 0;
unsigned long timerOnCurrentMillis     = 0;

const int STATE_INITIAL               = 0;
const int STATE_FIRST_TIME_ON         = 1;
const int STATE_FIRST_TIME_OFF        = 2;
const int STATE_SECOND_TIME_ON        = 3;
const int STATE_SECOND_TIME_OFF       = 4;
const int STATE_TIME_ON               = 5;
const int STATE_TIME_OFF              = 6;
int state                             = STATE_INITIAL;


void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  digitalWrite(relayPin, LOW);
}

void loop() {

  unsigned long currentMillis = millis();

  switch(state) {

    case STATE_INITIAL:
    break;
    case STATE_FIRST_TIME_ON:     //- ieslēdz katlu un sagaida, kad sāk šņākt.
    break;
    case STATE_FIRST_TIME_OFF:    //- palaiž OFF taimeri.

      if(timerOffCurrentMillis == 0) {
        timerOffCurrentMillis = currentMillis;
      }
      
    break;
    case STATE_SECOND_TIME_ON:    //- nofiksē un resetē OFF timeri un palaiž ON timeri.

      if(timerOffMillis == 0) {
        timerOffMillis = currentMillis - timerOffCurrentMillis;
        timerOffCurrentMillis = 0;
        timerOnCurrentMillis = currentMillis;
      }
      
    break;
    case STATE_SECOND_TIME_OFF: //- nofiksē un resetē ON timeri un palaiž OFF timeri.
      
      if(timerOnMillis == 0) {
        timerOnMillis = currentMillis - timerOnCurrentMillis;
        timerOnCurrentMillis = 0;
        timerOffCurrentMillis = currentMillis;
      }
      
    break;
    case STATE_TIME_ON: //- resetē OFF timeri un palaiž ON timeri.

      if(timerOffCurrentMillis > 0) {
        timerOffCurrentMillis = 0;
        timerOnCurrentMillis = currentMillis;
      }
    break;
    case STATE_TIME_OFF://- resetē ON timeri un palaiž OFF timeri.
      
      if(timerOnCurrentMillis > 0) {
        timerOnCurrentMillis = 0;
        timerOffCurrentMillis = currentMillis;
      }
    break;
    default:
    break;
  }

  // izslēdz katlu pēc ON timera iztecēšanas
  if(timerOnMillis > 0 && timerOnCurrentMillis > 0 && (currentMillis - timerOnCurrentMillis) >= timerOnMillis) {
    digitalWrite(relayPin, LOW);
    state = STATE_TIME_OFF;
  }

  // ieslēdz katlu pēc OFF timera iztecēšanas
  if(timerOffMillis > 0 && timerOffCurrentMillis > 0 && (currentMillis - timerOffCurrentMillis) >= timerOffMillis) {
    digitalWrite(relayPin, HIGH);
    state = STATE_TIME_ON;
  }

  buttonLoop();
}

void buttonLoop() {
   
  buttonState = !digitalRead(buttonPin);

  digitalWrite(ledPin, buttonState);
 
  // Process button state
  if (buttonState == HIGH) {

      if(previousButtonState == LOW) {
        previousButtonState = HIGH;

        digitalWrite(relayPin, !digitalRead(relayPin));

        if(state == STATE_INITIAL) {
          state = STATE_FIRST_TIME_ON;
        }
        else if(state == STATE_FIRST_TIME_OFF) {
          state = STATE_SECOND_TIME_ON;
        }
        else if ((state == STATE_TIME_OFF) || (state == STATE_SECOND_TIME_OFF)){
          state = STATE_TIME_ON;
        }
        else if(state == STATE_FIRST_TIME_ON) {
          state = STATE_FIRST_TIME_OFF;
        }
        else if(state == STATE_SECOND_TIME_ON) {
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
