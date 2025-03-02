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
  rotary.setMaxValue(100);
  Serial.println("Short Press will change Level");
  Serial.println("Long Press will change Application");
  state = ch_val;
}

void loop() {
  if (rotary.valChanged) {
    switch (state) {
      case ch_val: 
        Serial.printf("Appl: %u Level: %u Rotary Val: %u\n", rotary.curAppl(), rotary.curLevel(), rotary.curValue());
        break;
      case ch_lev:
        Serial.printf("Appl: %u Level: %u \n", rotary.curAppl(), rotary.curValue());
        break;
      case ch_app:
        Serial.printf("Appl: %u \n", rotary.curValue());
        break;
    }
    rotary.valChanged = false;
  }
  if (rotary.buttonChanged) {
    switch(state) {
      case ch_val: { 
        switch(rotary.buttonState()) {
          // short press
          case 1:
            state = ch_lev;
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


