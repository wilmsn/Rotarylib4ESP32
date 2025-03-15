#include "rotarylib4ESP32.h"

#define ROTARY_SW      32
#define ROTARY_A       27
#define ROTARY_B       26

RotaryLibMulti rotary;
//RotaryLib rotary;

void IRAM_ATTR intrSRV() {
  rotary.read();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Start");
  pinMode(ROTARY_A, INPUT_PULLUP);
  pinMode(ROTARY_B, INPUT_PULLUP);
  pinMode(ROTARY_SW, INPUT_PULLUP);
  attachInterrupt(ROTARY_A, intrSRV, CHANGE);
  attachInterrupt(ROTARY_B, intrSRV, CHANGE);
  attachInterrupt(ROTARY_SW, intrSRV, CHANGE);
  rotary.begin(ROTARY_A, ROTARY_B, ROTARY_SW);
  rotary.set_max(110);
  uint8_t my_app = 0;
  uint8_t my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_app = rotary.add_app();
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_app = rotary.add_app();
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  my_lev = rotary.add_lev(my_app);
  Serial.printf("my_app: %u; my_val: %u\n",my_app,my_lev);
  if (rotary.set(0,0,0,100,0)) Serial.println("App 0 Lev 1 set");
  if (rotary.set(0,1,0,90,10)) Serial.println("App 0 Lev 1 set");
  if (rotary.set(0,2,0,80,20)) Serial.println("App 0 Lev 2 set");
  if (rotary.set(0,3,0,70,30)) Serial.println("App 0 Lev 3 set");
  if (rotary.set(1,0,1,101,1)) Serial.println("App 1 Lev 1 set");
  if (rotary.set(1,1,1,91,11)) Serial.println("App 1 Lev 1 set");
  if (rotary.set(1,2,1,81,21)) Serial.println("App 1 Lev 2 set");
  if (rotary.set(1,3,1,71,31)) Serial.println("App 1 Lev 3 set");
  rotary.switch_app(1,0);
}

void loop() {
  if (rotary.valChanged) {
    Serial.printf("Rotary App: %u Lev: %u Val: %u\n", rotary.app(), rotary.lev(), rotary.val());
    rotary.valChanged = false;
  }
  if (rotary.buttonChanged) {
    switch(rotary.buttonState()) {
      case 1:
        Serial.println("Rotary Switch: short pressed");
        Serial.printf("App: %u Old Level is %u Val: %u Min: %u Max: %u\n", rotary.app(), rotary.lev(), rotary.val(), rotary.min(), rotary.max());
        if (rotary.lev() < rotary.max_lev()) {
          if (rotary.switch_lev(rotary.lev() +1)){
            Serial.printf("App: %u New Level is now %u Val: %u Min: %u Max: %u\n", rotary.app(), rotary.lev(), rotary.val(), rotary.min(), rotary.max());
          } else {
            Serial.println("Error switching Lev");
          }
        } else {
          if (rotary.switch_lev(0)) {
            Serial.printf("App: %u New Level is now %u Val: %u Min: %u Max: %u\n", rotary.app(), rotary.lev(), rotary.val(), rotary.min(), rotary.max());
          } else {
            Serial.println("Error switching Lev");
          }
        }

        break;
      case 2:
        Serial.println("Rotary Switch: long pressed");
        if (rotary.app() < rotary.max_app()) {
          if (rotary.switch_app(rotary.app() +1,0)) {
            Serial.printf("New App is now %u Lev: %u Val: %u Min: %u Max: %u\n", rotary.app(), rotary.lev(), rotary.val(), rotary.min(), rotary.max());
          } else {
            Serial.println("Error switching App");
          }
        } else {
          if (rotary.switch_app(0,0)) {
            Serial.printf("New App is now %u Lev: %u Val: %u Min: %u Max: %u\n", rotary.app(), rotary.lev(), rotary.val(), rotary.min(), rotary.max());
          } else {
            Serial.println("Error switching App");
          }
        }
        break;
    }
    rotary.buttonChanged = false;
  }
  rotary.read();
}


