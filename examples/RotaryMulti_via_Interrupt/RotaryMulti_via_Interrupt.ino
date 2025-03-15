/**************************************************
In this example we have these App(lications) and Lev(els):
App            Lev
0 Radio        0 Volume control
               1 Station select
               2 Application select

1 Mediaplayer  0 Volume control
               1 Album select
               2 Title select
               3 Application select

3 Settings     0 Bass settings
               1 Treble settings
               2 Media reload
               3 Application select

How to operate:
Short click increases level and commits the changed value (if any).
If it reaches the highest posible level it next short click will go to level 0
****************************************************/
#include "rotarylib4ESP32.h"

#define ROTARY_SW      32
#define ROTARY_A       27
#define ROTARY_B       26

RotaryLibMulti rotary;

enum state_t { ch_val, ch_lev, ch_app };
state_t state;

void IRAM_ATTR intrSRV() {
  rotary.read();
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Start");
  pinMode(ROTARY_A, INPUT_PULLUP);
  pinMode(ROTARY_B, INPUT_PULLUP);
  pinMode(ROTARY_SW, INPUT_PULLUP);
  attachInterrupt(ROTARY_A, intrSRV, CHANGE);
  attachInterrupt(ROTARY_B, intrSRV, CHANGE);
  attachInterrupt(ROTARY_SW, intrSRV, CHANGE);
  rotary.begin(ROTARY_A, ROTARY_B, ROTARY_SW);
  Serial.println("#1");
  // Add Applications and Levels
  // App=0; Lev=0  ==> Radio Volume Control
  //  => Added by default!!!
  // App=0; Lev=1  ==> Station Control
  uint8_t app = 0;
  uint8_t new_lev = 1;
  //uint8_t new_lev = rotary.add_lev(app);
  Serial.println("#1a");
  delay(1000);
  Serial.println("#2");
  Serial.printf("New Entry: App %u  Lev %u\n", app, new_lev);
  // App=1; Lev=0  ==> Mediaplayer Volume Control
  new_lev = 0;
  app = rotary.add_app();
  Serial.println("#3");
  Serial.printf("New Entry: App %u  Lev %u\n", app, new_lev);
  // App=1; Lev=1  ==> Mediaplayer Album Select
  new_lev = rotary.add_lev(app);
  Serial.println("#4");
  Serial.printf("New Entry: App %u  Lev %u\n", app, new_lev);
  // App=1; Lev=1  ==> Mediaplayer Song from Album Select
  new_lev = rotary.add_lev(app);
  Serial.printf("New Entry: App %u  Lev %u\n", app, new_lev);
  // App=2; Lev=0  ==> Settings Bass
  new_lev = 0;
  app = rotary.add_app();
  Serial.printf("New Entry: App %u  Lev %u\n", app, new_lev);
  // App=1; Lev=1  ==> Settings Treble
  new_lev = rotary.add_lev(app);
  Serial.printf("New Entry: App %u  Lev %u\n", app, new_lev);
  // App=1; Lev=1  ==> Settings Media Reload
  new_lev = rotary.add_lev(app);
  Serial.printf("New Entry: App %u  Lev %u\n", app, new_lev);

  // Set the Boundries and default values
  // Radio Volume Control
  rotary.set(0,0,0,100,0);
  // Radio Station Control
  rotary.set(0,1,0,20,0);

  // Mediaplayer Volume Control
  rotary.set(1,0,0,100,0);
  // Mediaplayer Select Album
  rotary.set(1,1,0,75,0);
  // Mediaplayer Select Song from Album
  rotary.set(1,2,0,15,0);

  // Settings Bass
  rotary.set(2,0,0,100,0);
  // Settings Treble
  rotary.set(2,1,0,100,0);
  // Settings Media Reload
  rotary.set(2,2,0,1,0);

  state = ch_val;
  Serial.println("Short Press will change Level");
  Serial.println("Long Press will change Application");
}

void loop() {
  if (rotary.valChanged) {
    Serial.printf("Appl: %u Level: %u Rotary Val: %u\n", rotary.app(), rotary.lev(), rotary.val());
    rotary.valChanged = false;
  }
  if (rotary.buttonChanged) {
    switch(state) {
      case ch_val: { 
        switch(rotary.buttonState()) {
          // short press
          case 1:
            state = ch_lev;

            if (rotary.lev() < rotary.max_lev()) rotary.switch_lev(rotary.lev() +1);
            Serial.printf("Select Level, short press to commit \n");
            break;
          // long press
          case 2:
            state = ch_app;
            Serial.printf("Select Application, short press to commit \n");
            break;
        }
      }
      break;
      case ch_lev: { 
        switch(rotary.buttonState()) {
          // short press
          case 1:
            state = ch_val;
            Serial.printf("commited \n");
            break;
        }
      }
      break;
      case ch_app: { 
        switch(rotary.buttonState()) {
          // short press
          case 1:
            state = ch_val;
            Serial.printf("commited \n");
            break;
        }
      }
      break;
    }
    rotary.buttonChanged = false;
  }
}


