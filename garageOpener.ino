#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>


#define RELAY 2
#define HALL_OPEN 7
#define HALL_CLOSED 8
#define LIGHT 9
#define INSIDE_BUTTON 10
#define MOTION_SENSOR 15
const long minute = 60000L;
const long closeWarning = minute*.2;
const long closeMax = minute*.4;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, LIGHT);

boolean triedAutoShutting = false;
boolean doorMoving = false;


long lastRelayOn = 0;
int relayDelay = 200;

boolean closingSoon = false;
long openTime = 0;
long lastMovement = 0;

Bounce insideButton = Bounce();

void setup() {
  Serial.begin(59700);

  pinMode(INSIDE_BUTTON, INPUT_PULLUP);
  insideButton.attach(INSIDE_BUTTON);
  insideButton.interval(20);


  pinMode(HALL_CLOSED, INPUT);
  pinMode(HALL_OPEN, INPUT);
  pinMode(RELAY, OUTPUT);

  //make sure it never triggers on powerup
  digitalWrite(RELAY, HIGH);

  strip.begin();
  strip.show();
}

void loop() {
  checkForMovement();
  insideButton.update();
  //show the state of the door
  updateStatusLed();


  if (insideButton.fell()){
    triggerRelay();
  }

  if (doorClosed()){
   openTime = 0;
   closingSoon = false;
   triedAutoShutting = false;
  }

  if (!doorClosed() && !doorMoving2()) {
    if (openTime == 0) {
     openTime = millis();
     Serial.println("door just opened Setting openTime");
     Serial.println("");
   } else if (millis() > (openTime + (closeMax)) && !triedAutoShutting){
      Serial.println("shutting door");
      Serial.println();
      triggerRelay();
      triedAutoShutting = true;
    } else if (millis() > (openTime + (closeWarning))){
      closingSoon = true;
    } else {
      closingSoon = false;
    }

  }

  clearRelay();
}

void checkForMovement() {
  if (digitalRead(MOTION_SENSOR) == HIGH) {
    lastMovement = millis();
    if (openTime != 0){
      openTime = lastMovement;
    }
  }
}

int brightness = 1;
int fadeAmount = 1;

void updateStatusLed(){
  if (millis() % 12 == 0){

    if (triedAutoShutting && !doorMoving2()){
      strip.setPixelColor(0, 255,0,0);
    } else if (!doorMoving2()){
      strip.setBrightness(255);
      if (!doorClosed()){
        if(closingSoon){
          strip.setPixelColor(0, 0,0,255);
        } else {
          strip.setPixelColor(0, 255,255,255);
        }
      } else if (doorClosed()){
        strip.setPixelColor(0, 0,0,0);
      }
    }
    else {
      strip.setPixelColor(0,0, 255,0);
      strip.setBrightness(brightness);

      // change the brightness for next time through the loop:
      brightness = brightness + fadeAmount;

      // reverse the direction of the fading at the ends of the fade:
      if (brightness == 1 || brightness == 255) {
        fadeAmount = -fadeAmount ;
      }
    }

    strip.show();
  }
}


void triggerRelay(){

  digitalWrite(RELAY, LOW);
  lastRelayOn = millis();
}


void clearRelay(){
  if ((millis() - lastRelayOn) > relayDelay) {
    digitalWrite(RELAY, HIGH);
  }

}

boolean doorClosed(){
  return digitalRead(HALL_CLOSED) == HIGH;
}

boolean doorOpen(){
  return digitalRead(HALL_OPEN) == HIGH;
}

boolean doorMoving2() {
  return (!doorOpen() && !doorClosed()) || lastRelayOn + 3000 > millis();
}
