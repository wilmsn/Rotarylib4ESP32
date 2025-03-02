#include "rotarylib4ESP32.h"

#define ROTARY_SW      32
#define ROTARY_A       27
#define ROTARY_B       26

RotaryLib rotary;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Start");
	pinMode(ROTARY_A, INPUT_PULLUP);
	pinMode(ROTARY_B, INPUT_PULLUP);
	pinMode(ROTARY_SW, INPUT_PULLUP);
  rotary.begin(ROTARY_A, ROTARY_B, ROTARY_SW);
  rotary.set_max(100);
}

void loop() {
  if (rotary.valChanged) {
    Serial.printf("Rotary Val: %u\n", rotary.val());
    rotary.valChanged = false;
  }
  if (rotary.buttonChanged) {
    switch(rotary.buttonState()) {
      case 1:
        Serial.println("Rotary Switch: short pressed");
        break;
      case 2:
        Serial.println("Rotary Switch: long pressed");
        break;
    }
    rotary.buttonChanged = false;
  }
  rotary.read();
}


